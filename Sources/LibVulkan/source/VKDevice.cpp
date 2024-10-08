/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"

#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKEnvironment.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSubmitInfo.h"
#include "LibVulkan/include/VKSurface.h"

namespace Vulkan
{



Device::Device() :
    _device(VK_NULL_HANDLE),
    _queueGraphic(VK_NULL_HANDLE),
    _colorFormat(VK_FORMAT_UNDEFINED),
    _depthFormat(VK_FORMAT_UNDEFINED),
    _rayTracingPipelineProperties({VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR}),
    _accelerationStructureFeatures({VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR})
{
    MouCa::preCondition(isNull());			                //Dev Issue: Bad constructor !
}

Device::~Device()
{
    MouCa::preCondition(isNull());			                //Dev Issue: Missing release()
    MouCa::preCondition(_queueGraphic == VK_NULL_HANDLE);	//Dev Issue: Missing release()
}

void Device::initialize(const VkPhysicalDevice physicalDevice, const uint32_t queueFamilyID, const std::vector<const char*>& extensions)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(physicalDevice != VK_NULL_HANDLE);

    //Search Queue families
    uint32_t nbQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nbQueueFamilies, nullptr);

    //Read queue family properties
    _queueFamilyProperties.resize(static_cast<size_t>(nbQueueFamilies));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nbQueueFamilies, _queueFamilyProperties.data());

    // Get physical device properties
    vkGetPhysicalDeviceProperties(physicalDevice, &_properties);

    VkPhysicalDeviceProperties2 properties
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
         &_rayTracingPipelineProperties,
        {}
    };
    vkGetPhysicalDeviceProperties2(physicalDevice, &properties);
    VkPhysicalDeviceFeatures2 features2
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        &_accelerationStructureFeatures,
        {}
    };
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

    //Search physical feature
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    _queueFamilyIndices._graphics = getQueueFamiliyIndex(VK_QUEUE_GRAPHICS_BIT);

    _queueFamilyIndices._compute  = getQueueFamiliyIndex(VK_QUEUE_COMPUTE_BIT);

    _queueFamilyIndices._transfer = getQueueFamiliyIndex(VK_QUEUE_TRANSFER_BIT);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    const std::vector<float> queuePriorities = {0.0f};

    const VkDeviceQueueCreateInfo queueCreateInfo =
    {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
        nullptr,                                        // const void                  *pNext
        0,                                              // VkDeviceQueueCreateFlags     flags
        _queueFamilyIndices._graphics,					// uint32_t                     queueFamilyIndex
        static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
        queuePriorities.data()                          // const float                 *pQueuePriorities
    };
    queueCreateInfos.push_back(queueCreateInfo);

    if(_queueFamilyIndices._graphics != _queueFamilyIndices._compute)
    {
        const VkDeviceQueueCreateInfo queueGraphicCreateInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
            nullptr,                                        // const void                  *pNext
            0,                                              // VkDeviceQueueCreateFlags     flags
            _queueFamilyIndices._compute,					// uint32_t                     queueFamilyIndex
            static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
            queuePriorities.data()                          // const float                 *pQueuePriorities
        };
        queueCreateInfos.push_back(queueGraphicCreateInfo);
    }
    if(_queueFamilyIndices._transfer != _queueFamilyIndices._graphics)
    {
        const VkDeviceQueueCreateInfo queueTransferCreateInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
            nullptr,                                        // const void                  *pNext
            0,                                              // VkDeviceQueueCreateFlags     flags
            _queueFamilyIndices._transfer,					// uint32_t                     queueFamilyIndex
            static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
            queuePriorities.data()                          // const float                 *pQueuePriorities
        };
        queueCreateInfos.push_back(queueTransferCreateInfo);
    }

    //Check extensions if wanted by user
    const char * const * ptrExtensions = nullptr;
    if(!extensions.empty())
    {
        checkExtensions(physicalDevice, extensions);
        ptrExtensions = extensions.data();
    }

    VkDeviceCreateInfo deviceCreateInfo
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // VkStructureType                    sType
        nullptr,                                        // const void                        *pNext
        0,                                              // VkDeviceCreateFlags                flags
        static_cast<uint32_t>(queueCreateInfos.size()), // uint32_t                           queueCreateInfoCount
        queueCreateInfos.data(),                        // const VkDeviceQueueCreateInfo     *pQueueCreateInfos
        0,                                              // uint32_t                           enabledLayerCount
        nullptr,                                        // const char * const                *ppEnabledLayerNames
        static_cast<uint32_t>(extensions.size()),       // uint32_t                           enabledExtensionCount
        ptrExtensions,									// const char * const                *ppEnabledExtensionNames
        &features                                       // const VkPhysicalDeviceFeatures    *pEnabledFeatures
    };

    //Build feature v2 (support of raytracing)
    const VkPhysicalDeviceFeatures2 physicalDeviceFeatures2
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        _enabled.getNext(),
        _enabled._features,
    };
    
    // Enable raytracing
    if(std::find(extensions.cbegin(), extensions.cend(), VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) != extensions.cend())
    {
        deviceCreateInfo.pNext            = &physicalDeviceFeatures2;
        deviceCreateInfo.pEnabledFeatures = nullptr;
    }

    // Build Device
    VkDevice deviceID;
    if(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &deviceID) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("VulkanError", "DeviceError"));
    }

    configureDevice(physicalDevice, deviceID, queueFamilyID);

#ifdef VULKAN_DEBUG
    //Check layers installed
    uint32_t nbLayersProperties = 0;
    if(vkEnumerateDeviceLayerProperties(physicalDevice, &nbLayersProperties, nullptr) != VK_SUCCESS && nbLayersProperties > 0)
    {
        throw Core::Exception(Core::ErrorData("VulkanError", "NbLayersError"));
    }
    std::vector<VkLayerProperties> layerProperties(nbLayersProperties);
    if(vkEnumerateDeviceLayerProperties(physicalDevice, &nbLayersProperties, layerProperties.data())  != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("VulkanError", "NbLayersError"));
    }
    std::cout << "Vulkan - Physical Layers:" << std::endl;
    for(const auto& layer: layerProperties)
    {
        std::cout << "\t" << layer.layerName << ": " << layer.description << std::endl;
    }
#endif

    // Load Extensions
    vkGetBufferDeviceAddressKHR                 = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(deviceID, "vkGetBufferDeviceAddressKHR"));
    vkCmdBuildAccelerationStructuresKHR         = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(deviceID, "vkCmdBuildAccelerationStructuresKHR"));
    vkBuildAccelerationStructuresKHR            = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(deviceID, "vkBuildAccelerationStructuresKHR"));
    vkCreateAccelerationStructureKHR            = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(deviceID, "vkCreateAccelerationStructureKHR"));
    vkDestroyAccelerationStructureKHR           = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(deviceID, "vkDestroyAccelerationStructureKHR"));
    vkGetAccelerationStructureBuildSizesKHR     = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(deviceID, "vkGetAccelerationStructureBuildSizesKHR"));
    vkGetAccelerationStructureDeviceAddressKHR  = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(deviceID, "vkGetAccelerationStructureDeviceAddressKHR"));
    vkCmdTraceRaysKHR                           = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(deviceID, "vkCmdTraceRaysKHR"));
    vkGetRayTracingShaderGroupHandlesKHR        = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(deviceID, "vkGetRayTracingShaderGroupHandlesKHR"));
    vkCreateRayTracingPipelinesKHR              = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(deviceID, "vkCreateRayTracingPipelinesKHR"));

    MouCa::postCondition(!isNull());
}

void Device::configureDevice(const VkPhysicalDevice physicalDevice, const VkDevice deviceID, const uint32_t queueFamilyID)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(physicalDevice != VK_NULL_HANDLE);
    MouCa::preCondition(deviceID != VK_NULL_HANDLE);

    //Set current id
    _physicalDevice = physicalDevice;
    _device         = deviceID;

    vkGetDeviceQueue(_device, _queueFamilyIndices._graphics, 0, &_queueGraphic);

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &_memoryProperties);
    //vkGetPhysicalDeviceFeatures(physicalDevice, &_enabledFeatures);

    //------ Best color format ------------------------------------------------------------------------------
    _colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

    //------ Search best depth format -----------------------------------------------------------------------
    const std::vector<VkFormat> depthFormats =
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for(auto& format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            _depthFormat = format;
            break;
        }
    }
    MouCa::postCondition(_depthFormat != VK_FORMAT_UNDEFINED);
    MouCa::postCondition(!isNull());
}

void Device::initializeBestGPU(const Environment& environment, const std::vector<const char*>& extensions, const Surface* surface, const PhysicalDeviceFeatures& enabled)
{
    MouCa::preCondition(!environment.isNull());
    MouCa::preCondition(isNull());
    MouCa::preCondition(surface == nullptr || !surface->isNull());

    _enabled = enabled;

    std::set<DeviceCandidate, DeviceCandidate::Less> canditates;

    //Select candidate
    for(auto physicalDevice : environment.getGraphicsPhysicalDevices())
    {
        DeviceCandidate candidate;
        candidate.deviceID = physicalDevice;

        //Read physical device property
        vkGetPhysicalDeviceProperties(physicalDevice, &candidate.properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &candidate.features);

        if(candidate.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            continue;
        }

        // Check extensions are fully supported
        if(!extensions.empty())
        {
            try
            {
                checkExtensions(physicalDevice, extensions);
            }
            catch(const Core::Exception&)
            {
                continue;
            }
        }

        // Check supported features and mandatory
        try
        {
            // Parse feature like array of bool (Need to control we can do that all the time !)
            // Possible crash/bug during parsing if Vulkan API evolves.
            const VkBool32* current = reinterpret_cast<const VkBool32*>(&candidate.features);
            const VkBool32* wanted  = reinterpret_cast<const VkBool32*>(&enabled);
            
            for(uint32_t id = 0; id < sizeof(candidate.features)/sizeof(VkBool32); ++id)
            {
                // Check if feature is mandatory and can be use on physical device
                if(*wanted && *wanted != *current)
                {
                    throw Core::Exception(Core::ErrorData("VulkanError", "UnsupportedPhysicalDeviceFeatures") << std::to_string(id));
                }
                ++wanted;
                ++current;
            }
        }
        catch (const Core::Exception&)
        {
            continue;
        }

        //Search Queue families
        uint32_t nbQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nbQueueFamilies, nullptr);

        //Read queue family properties
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(nbQueueFamilies);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nbQueueFamilies, queueFamilyProperties.data());

        // Choose first Graphic ID
        uint32_t queueFamilyID = 0;
        for(auto queueFamilyProperty : queueFamilyProperties)
        {
            VkBool32 supportSurface = true;
            if(surface != nullptr)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyID, surface->getInstance(), &supportSurface);
            }

            // Try to find best queueID
            if(supportSurface && (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
            {
                break; // We found it: quit for loop !
            }
            ++queueFamilyID;
        }

        // Finish selection
        if(queueFamilyProperties.size() > queueFamilyID)
        {
            candidate.queueFamilyID = queueFamilyID;
            canditates.insert(candidate);
        }
    }

    //Finish
    if(canditates.empty())
    {
        throw Core::Exception(Core::ErrorData("VulkanError", "BestDeviceError"));
    }

    // Initialize device
    initialize(canditates.begin()->deviceID, canditates.begin()->queueFamilyID, extensions);

    MouCa::postCondition(!isNull());
}

void Device::release()
{
    MouCa::assertion(!isNull());

    waitIdle();

    //Release device
    vkDestroyDevice(_device, nullptr);
        
    _device       = VK_NULL_HANDLE;
    _queueGraphic = VK_NULL_HANDLE;

    MouCa::postCondition(isNull());
}

void Device::waitIdle() const
{
    MouCa::preCondition(!isNull());

    //Finish current task
    if( vkDeviceWaitIdle(_device) != VK_SUCCESS)
    {
// DisableCodeCoverage
        throw Core::Exception(Core::ErrorData("VulkanError", "LockedDevice"));
// EnableCodeCoverage
    }
}

void Device::checkExtensions(const VkPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions) const
{
    MouCa::preCondition(physicalDevice != VK_NULL_HANDLE);	//DEV Issue: Need to have a valid physical device !
    MouCa::preCondition(!extensions.empty());				//DEV Issue: need to check extension only if sent !

    //Read extensions supported by all
    uint32_t nbExtensions = 0;
    if(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nbExtensions, nullptr) != VK_SUCCESS)
    {
// DisableCodeCoverage
        // Impossible to reach ? vkEnumerateDeviceExtensionProperties launch exception before !?
        throw Core::Exception(Core::ErrorData("VulkanError", "PhysicalDeviceExtensionError"));
// EnableCodeCoverage
    }
    std::vector<VkExtensionProperties> availableExtensions(nbExtensions);
    if(nbExtensions == 0 || vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nbExtensions, availableExtensions.data()) != VK_SUCCESS)
    {
// DisableCodeCoverage
        // Impossible to reach ?
        throw Core::Exception(Core::ErrorData("VulkanError", "PhysicalDeviceExtensionError"));
// EnableCodeCoverage
    }

    //Define lambda
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

    //Check extension
    for(auto extension : extensions)
    {
        if(!compare(extension))
        {
            throw Core::Exception(Core::ErrorData("VulkanError", "MissingPhysicalDeviceExtensionError") << extension);
        }
    }
}

uint32_t Device::getMemoryType(const VkMemoryRequirements& memoryRequiered, const VkMemoryPropertyFlags properties) const
{
    uint32_t typeBits = memoryRequiered.memoryTypeBits;
    for(uint32_t i = 0; i < _memoryProperties.memoryTypeCount; i++)
    {
        if((typeBits & 1) == 1)
        {
            if((_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }      
    return 0;
}

uint32_t Device::getQueueFamiliyIndex(VkQueueFlagBits queueFlags) const
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if(queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for(uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
        {
            if((_queueFamilyProperties[i].queueFlags & queueFlags) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if(queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
        for(uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
        {
            if((_queueFamilyProperties[i].queueFlags & queueFlags) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for(uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
    {
        if(_queueFamilyProperties[i].queueFlags & queueFlags)
        {
            return i;
        }
    }

    throw Core::Exception(Core::ErrorData("Vulkan", "FamilyQueueInvalidError"));
}

void Device::readFormatProperties(const VkFormat format, VkFormatProperties& formatProps) const
{
    MouCa::assertion(_physicalDevice != VK_NULL_HANDLE);        //DEV Issue: Missing initialize ?
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &formatProps);
}



void Device::executeCommandSync(std::vector<ICommandBufferWPtr>&& commandBuffers) const
{
    // Build submit data
    SubmitInfos infos;
    infos.resize(1);
    infos[0] = std::make_unique<SubmitInfo>();
    infos[0]->initialize(std::move(commandBuffers));

    // Build Fence
    auto fence = std::make_shared<Fence>();
    fence->initialize(*this, Fence::infinityTimeout, 0);

    // Execute sequence
    {
        // Submit
        SequenceSubmit submit(std::move(infos), fence);
        submit.execute(*this);

        // Wait fence
        const std::vector<Vulkan::FenceWPtr> fences{ fence };
        SequenceWaitFence waitFence(fences, Fence::infinityTimeout, VK_TRUE);
        waitFence.execute(*this);

        // Sync device
        //device.waitIdle();
    }

    fence->release(*this);
}

}