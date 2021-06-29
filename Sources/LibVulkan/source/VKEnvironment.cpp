/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibRT/include/RTEnum.h>

#include "LibVulkan/include/VKEnvironment.h"

#define DEBUG_LAYERS

namespace Vulkan
{

Environment::Environment():
_instance( VK_NULL_HANDLE )
{}

Environment::~Environment()
{
    MOUCA_ASSERT(isNull());
}

void Environment::initialize(const RT::ApplicationInfo& info, const std::vector<const char*>& extensions)
{
    MOUCA_ASSERT(_graphicsDevices.empty());	//DEV Issue: second call ?
    MOUCA_ASSERT(_rejectDevices.empty());		//DEV Issue: second call ?
    MOUCA_ASSERT(_computeDevices.empty());		//DEV Issue: second call ?
    MOUCA_ASSERT(isNull());	                //DEV Issue: second call ?

    VkApplicationInfo application_info =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,             // VkStructureType            sType
        nullptr,                                        // const void                *pNext
        info._applicationName.c_str(),                  // const char                *pApplicationName
        info._applicationVersion,   					// uint32_t                   applicationVersion
        info._engineName.c_str(),						// const char                *pEngineName
        info._engineVersion,							// uint32_t                   engineVersion
        VK_API_VERSION_1_2                              // uint32_t                   apiVersion
    };

#ifdef VULKAN_DEBUG
    // Get all extensions
    std::vector<const char*> layers;
    std::vector<const char*> myExtensions = extensions;

#   ifdef DEBUG_LAYERS
    layers = DebugReport::getLayers();
    myExtensions.push_back(_report.getExtension());
#   endif

    //Check extensions if wanted by user
    if(!myExtensions.empty())
    {
        checkExtensions(myExtensions);
    }

    VkInstanceCreateInfo instance_create_info =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         // VkStructureType            sType
        nullptr,                                        // const void*                pNext
        0,                                              // VkInstanceCreateFlags      flags
        &application_info,                              // const VkApplicationInfo   *pApplicationInfo
        static_cast<uint32_t>(layers.size()),           // uint32_t                   enabledLayerCount
        !layers.empty() ? layers.data() : nullptr,      // const char * const        *ppEnabledLayerNames
        static_cast<uint32_t>(myExtensions.size()),		// uint32_t                   enabledExtensionCount
        myExtensions.data()                             // const char * const        *ppEnabledExtensionNames
    };
#else
    //Check extensions if wanted by user
    if(!extensions.empty())
    {
        checkExtensions(extensions);
    }

    VkInstanceCreateInfo instance_create_info =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         // VkStructureType            sType
        nullptr,                                        // const void*                pNext
        0,                                              // VkInstanceCreateFlags      flags
        &application_info,                              // const VkApplicationInfo   *pApplicationInfo
        0,                                              // uint32_t                   enabledLayerCount
        nullptr,                                        // const char * const        *ppEnabledLayerNames
        static_cast<uint32_t>(extensions.size()),		// uint32_t                   enabledExtensionCount
        extensions.data()                               // const char * const        *ppEnabledExtensionNames
    };

#endif

    //Generate instance
    const VkResult error = vkCreateInstance(&instance_create_info, nullptr, &_instance);
    if( error != VK_SUCCESS )
    {
        MOUCA_THROW_ERROR(u8"VulkanError", error == VK_ERROR_LAYER_NOT_PRESENT ? u8"InstanceLayoutError" : u8"InstanceError");
    }

#ifdef VULKAN_DEBUG
#   ifdef DEBUG_LAYERS
    _report.initialize(_instance);
#   endif DEBUG_LAYERS

    //Check layers installed
    uint32_t nbLayersProperties = 0;
    if(vkEnumerateInstanceLayerProperties(&nbLayersProperties, nullptr) != VK_SUCCESS && nbLayersProperties > 0)
    {
        MOUCA_THROW_ERROR(u8"VulkanError", u8"NbLayersError");
    }
    std::vector<VkLayerProperties> properties(nbLayersProperties);
    if(vkEnumerateInstanceLayerProperties(&nbLayersProperties, properties.data())  != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"VulkanError", u8"NbLayersError");
    }
    std::cout << "Vulkan - Layers:" << std::endl;
    for(const auto& layer: properties)
    {
        std::cout << "\t" << layer.layerName << ": " << layer.description << std::endl;
    }
#endif

    //Search number of PhysicalDevices
    uint32_t nbPhysicalDevices = 0;
    if(vkEnumeratePhysicalDevices(_instance, &nbPhysicalDevices, nullptr) != VK_SUCCESS && nbPhysicalDevices > 0)
    {
        MOUCA_THROW_ERROR(u8"VulkanError", u8"NbPhysicalDevicesError");
    }

    //Resize vector to keep each device info
    PhysicalDevices physicalDevices(nbPhysicalDevices);

    //Search PhysicalDevices
    if(vkEnumeratePhysicalDevices(_instance, &nbPhysicalDevices, physicalDevices.data()) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"VulkanError", u8"PhysicalDevicesError");
    }

    //Parse device to sort between graphic/compute (quick classification)
    for(auto device : physicalDevices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures   deviceFeatures;
    
        //Read physical device property
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        //Search Queue families
        uint32_t nbQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &nbQueueFamilies, nullptr);

        bool valid   = deviceProperties.apiVersion >= VK_API_VERSION_1_0 && nbQueueFamilies > 0;
        bool graphic = false;
        if(valid)
        {
            //Read queue family properties
            std::vector<VkQueueFamilyProperties> queueFamilyProperties(nbQueueFamilies);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &nbQueueFamilies, &queueFamilyProperties[0]);

            for(auto queueFamilyProperty : queueFamilyProperties)
            {
                valid   = valid   && (queueFamilyProperty.queueCount > 0);
                graphic = graphic || (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT;
            }
        }
            
        //Memorize device
        if(valid)
        {
            if(graphic)
            {
                _graphicsDevices.push_back(device);
            }
            else
            {
                _computeDevices.push_back(device);
            }
        }
        else
        {
            _rejectDevices.push_back(device);
        }
    }

    //Final check to see if we not loose data
    MOUCA_POST_CONDITION(_rejectDevices.size() + _graphicsDevices.size() + _computeDevices.size() == physicalDevices.size());
    MOUCA_POST_CONDITION(!isNull());
}

void Environment::release()
{
    MOUCA_PRE_CONDITION(!isNull());
    //Clean context and associated device

    //Lost physical devices
    _rejectDevices.clear();
    _graphicsDevices.clear();
    _computeDevices.clear();

#ifdef VULKAN_DEBUG
    _report.release(_instance);
#endif

    //Release instance
    MOUCA_ASSERT(_instance != VK_NULL_HANDLE); //DEV Issue: release before initialize ?
    vkDestroyInstance(_instance, nullptr);
    _instance = VK_NULL_HANDLE;
    
    MOUCA_PRE_CONDITION(isNull());
}

void Environment::checkExtensions(const std::vector<const char*>& extensions) const
{
    MOUCA_ASSERT(!extensions.empty()); //DEV Issue: need to check extension only if sent !

    //Read extensions supported by all
    uint32_t nbExtensions = 0;
    if(vkEnumerateInstanceExtensionProperties(nullptr, &nbExtensions, nullptr) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"VulkanError", u8"ExtensionError");
    }
    std::vector<VkExtensionProperties> availableExtensions(nbExtensions);
    if(nbExtensions==0 || vkEnumerateInstanceExtensionProperties(nullptr, &nbExtensions, &availableExtensions[0]) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"VulkanError", u8"ExtensionError");
    }

    //Define lambda
// DisableCodeCoverage
    const auto compare = [&](char const*const currentExtension) -> bool
    {
        for(auto parsedExtension : availableExtensions)
        {
            if(strcmp(parsedExtension.extensionName, currentExtension) == 0)
            {
                return true;
            }
        }
        return false;
    };
// EnableCodeCoverage

    for(auto extension : extensions)
    {
        if(!compare(extension))
        {
            MOUCA_THROW_ERROR_1(u8"VulkanError", u8"MissingExtensionError", extension);
        }
    }
}

}