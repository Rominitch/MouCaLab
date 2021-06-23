#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DXMLLoader.h"
#include "MouCaGraphicEngine/include/Engine3DXMLHelper.h"

#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKContextDevice.h>

#include "MouCaGraphicEngine/include/GraphicEngine.h"
#include "MouCaGraphicEngine/include/VulkanEnum.h"
#include "MouCaGraphicEngine/include/VulkanManager.h"

namespace MouCaGraphic
{

void Engine3DXMLLoader::loadEnvironment(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(context._parser.isLoaded());           //DEV Issue: Need a valid xml.
    MOUCA_PRE_CONDITION(_manager.getEnvironment().isNull());    //DEV Issue: Environment MUST be not configured.

    // Check node exists
    auto result = context._parser.getNode(u8"Environment");
    if (result->getNbElements() == 0)
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"LoadingXMLCorruptError", context.getFileName(), u8"Environment");
    }

    // Go into
    XML::NodeUPtr environmentNode = result->getNode(0);
    
    // Read App info
    environmentNode->getAttribute(u8"application", context._info._applicationName);
    environmentNode->getAttribute(u8"engine", context._info._engineName);

    // cppcheck-suppress unreadVariable // false positive
    auto aPush = context._parser.autoPushNode(*environmentNode);

    // Get extensions: WARNING extensions must exists to validate cExtensions;
    std::vector<Core::String> extensions;

    // Check if we need to load VR support
    auto& vrPlatform = context._engine.getVRPlatform();
    if (!vrPlatform.isNull())
    {
        extensions = vrPlatform.getInstanceExtensions();
    }

    loadExtensions(context, extensions);

    std::vector<const char*> cExtensions;
    LoaderHelper::createCharExtensions(extensions, cExtensions);

    // Build environment
    _manager.getEnvironment().initialize(context._info, cExtensions);

    MOUCA_POST_CONDITION(!_manager.getEnvironment().isNull());  //DEV Issue: Environment MUST be configured.
}

void Engine3DXMLLoader::loadExtensions(ContextLoading& context, std::vector<Core::String>& extensions)
{
    // Read extensions
    auto result = context._parser.getNode(u8"Extension");

    // Allocate more space
    extensions.reserve(extensions.size() + result->getNbElements());

    for (size_t idExtension = 0; idExtension < result->getNbElements(); ++idExtension)
    {
        Core::String osName;
        auto extension = result->getNode(idExtension);
        if (extension->hasAttribute(u8"os"))
        {
            extension->getAttribute(u8"os", osName);
        }
        else // When attribute is not present consider like current os to add it.
        {
            osName = VulkanManager::_osName;
        }

        if (osName == VulkanManager::_osName)
        {
            Core::String extensionValue;
            extension->getValue(extensionValue);
            extensions.emplace_back(std::move(extensionValue));
        }
    }
}

void Engine3DXMLLoader::loadPhysicalDeviceFeatures(ContextLoading& context, Vulkan::PhysicalDeviceFeatures& mandatory)
{
    const auto toVKBool = [](bool e) {return e ? VK_TRUE : VK_FALSE; };
    bool enable;
    // Read extensions
    auto result = context._parser.getNode(u8"FeatureBufferDeviceAddress");
    if(result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        featureNode->getAttribute(u8"bufferDeviceAddress", enable);

        mandatory._bufferDeviceAddresFeatures.bufferDeviceAddress = toVKBool(enable);
    }

    result = context._parser.getNode(u8"FeatureRayTracingPipeline");
    if (result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        featureNode->getAttribute(u8"rayTracingPipeline", enable);

        mandatory._rayTracingPipelineFeatures.rayTracingPipeline = toVKBool(enable);
    }

    result = context._parser.getNode(u8"FeatureAccelerationStructure");
    if (result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        featureNode->getAttribute(u8"accelerationStructure", enable);

        mandatory._accelerationStructureFeatures.accelerationStructure = toVKBool(enable);
    }

    result = context._parser.getNode(u8"Features");
    if (result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        
        const auto enableFeature = [&](const Core::String& idNode, VkBool32& feature)
        {
            if (featureNode->hasAttribute(idNode))
            {
                featureNode->getAttribute(idNode, enable);
                feature = toVKBool(enable);
            }
        };

        enableFeature(u8"alphaToOne",                               mandatory._features.alphaToOne);
        enableFeature(u8"depthBiasClamp",                           mandatory._features.depthBiasClamp);
        enableFeature(u8"depthBounds",                              mandatory._features.depthBounds);
        enableFeature(u8"depthClamp",                               mandatory._features.depthClamp);
        enableFeature(u8"drawIndirectFirstInstance",                mandatory._features.drawIndirectFirstInstance);
        enableFeature(u8"dualSrcBlend",                             mandatory._features.dualSrcBlend);
        enableFeature(u8"fillModeNonSolid",                         mandatory._features.fillModeNonSolid);
        enableFeature(u8"fragmentStoresAndAtomics",                 mandatory._features.fragmentStoresAndAtomics);
        enableFeature(u8"fullDrawIndexUint32",                      mandatory._features.fullDrawIndexUint32);
        enableFeature(u8"geometryShader",                           mandatory._features.geometryShader);
        enableFeature(u8"imageCubeArray",                           mandatory._features.imageCubeArray);
        enableFeature(u8"independentBlend",                         mandatory._features.independentBlend);
        enableFeature(u8"inheritedQueries",                         mandatory._features.inheritedQueries);
        enableFeature(u8"largePoints",                              mandatory._features.largePoints);
        enableFeature(u8"samplelogicOprAnisotropy",                 mandatory._features.logicOp);
        enableFeature(u8"multiDrawIndirect",                        mandatory._features.multiDrawIndirect);
        enableFeature(u8"multiViewport",                            mandatory._features.multiViewport);
        enableFeature(u8"occlusionQueryPrecise",                    mandatory._features.occlusionQueryPrecise);
        enableFeature(u8"pipelineStatisticsQuery",                  mandatory._features.pipelineStatisticsQuery);
        enableFeature(u8"robustBufferAccess",                       mandatory._features.robustBufferAccess);
        enableFeature(u8"samplerAnisotropy",                        mandatory._features.samplerAnisotropy);
        enableFeature(u8"sampleRateShading",                        mandatory._features.sampleRateShading);
        enableFeature(u8"shaderClipDistance",                       mandatory._features.shaderClipDistance);
        enableFeature(u8"shaderCullDistance",                       mandatory._features.shaderCullDistance);
        enableFeature(u8"shaderFloat64",                            mandatory._features.shaderFloat64);
        enableFeature(u8"shaderImageGatherExtended",                mandatory._features.shaderImageGatherExtended);
        enableFeature(u8"shaderInt16",                              mandatory._features.shaderInt16);
        enableFeature(u8"shaderInt64",                              mandatory._features.shaderInt64);
        enableFeature(u8"shaderResourceMinLod",                     mandatory._features.shaderResourceMinLod);
        enableFeature(u8"shaderResourceResidency",                  mandatory._features.shaderResourceResidency);
        enableFeature(u8"shaderSampledImageArrayDynamicIndexing",   mandatory._features.shaderSampledImageArrayDynamicIndexing);
        enableFeature(u8"shaderStorageBufferArrayDynamicIndexing",  mandatory._features.shaderStorageBufferArrayDynamicIndexing);
        enableFeature(u8"shaderStorageImageArrayDynamicIndexing",   mandatory._features.shaderStorageImageArrayDynamicIndexing);
        enableFeature(u8"shaderStorageImageExtendedFormats",        mandatory._features.shaderStorageImageExtendedFormats);
        enableFeature(u8"shaderStorageImageMultisample",            mandatory._features.shaderStorageImageMultisample);
        enableFeature(u8"shaderStorageImageReadWithoutFormat",      mandatory._features.shaderStorageImageReadWithoutFormat);
        enableFeature(u8"shaderStorageImageWriteWithoutFormat",     mandatory._features.shaderStorageImageWriteWithoutFormat);
        enableFeature(u8"shaderTessellationAndGeometryPointSize",   mandatory._features.shaderTessellationAndGeometryPointSize);
        enableFeature(u8"shaderUniformBufferArrayDynamicIndexing",  mandatory._features.shaderUniformBufferArrayDynamicIndexing);
        enableFeature(u8"sparseBinding",                            mandatory._features.sparseBinding);
        enableFeature(u8"sparseResidency16Samples",                 mandatory._features.sparseResidency16Samples);
        enableFeature(u8"sparseResidency2Samples",                  mandatory._features.sparseResidency2Samples);
        enableFeature(u8"sparseResidency4Samples",                  mandatory._features.sparseResidency4Samples);
        enableFeature(u8"sparseResidency8Samples",                  mandatory._features.sparseResidency8Samples);
        enableFeature(u8"sparseResidencyAliased",                   mandatory._features.sparseResidencyAliased);
        enableFeature(u8"sparseResidencyBuffer",                    mandatory._features.sparseResidencyBuffer);
        enableFeature(u8"sparseResidencyImage2D",                   mandatory._features.sparseResidencyImage2D);
        enableFeature(u8"sparseResidencyImage3D",                   mandatory._features.sparseResidencyImage3D);
        enableFeature(u8"tessellationShader",                       mandatory._features.tessellationShader);
        enableFeature(u8"textureCompressionASTC_LDR",               mandatory._features.textureCompressionASTC_LDR);
        enableFeature(u8"textureCompressionBC",                     mandatory._features.textureCompressionBC);
        enableFeature(u8"textureCompressionETC2",                   mandatory._features.textureCompressionETC2);
        enableFeature(u8"sparseBinding",                            mandatory._features.sparseBinding);
        enableFeature(u8"variableMultisampleRate",                  mandatory._features.variableMultisampleRate);
        enableFeature(u8"vertexPipelineStoresAndAtomics",           mandatory._features.vertexPipelineStoresAndAtomics);
        enableFeature(u8"wideLines",                                mandatory._features.wideLines);
    }
}

void Engine3DXMLLoader::loadDevices(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(context._parser.isLoaded()); //DEV Issue: Need a valid xml.

    auto result = context._parser.getNode(u8"Device");
    for (size_t idDevice = 0; idDevice < result->getNbElements(); ++idDevice)
    {
        auto device = result->getNode(idDevice);

        Vulkan::ContextDeviceWPtr ctxDevice;
        bool existing = false;
        const uint32_t id = LoaderHelper::getIdentifiant(device, u8"Device", _devices, context, existing);
        if (!existing)
        {
            Core::String mode;
            device->getAttribute(u8"mode", mode);
            if (mode == u8"render")
            {
                uint32_t idSurface = 0;
                device->getAttribute(u8"compatibleWindowId", idSurface);

                if (idSurface >= _manager.getSurfaces().size())
                {
                    MOUCA_THROW_ERROR_1(u8"Engine3D", u8"UnknownSurfaceError", context.getFileName());
                }

                auto aPush = context._parser.autoPushNode(*device);

                // Get extensions: WARNING extensions must exists to validate cExtensions;
                std::vector<Core::String> extensions;
                auto& vrPlatform = context._engine.getVRPlatform();
                if (!vrPlatform.isNull())
                {
                    extensions = vrPlatform.getPhysicalDeviceExtensions(vrPlatform.getVulkanPhysicalDevice(_manager.getEnvironment()));
                }
                loadExtensions(context, extensions);
                std::vector<const char*> cExtensions;
                LoaderHelper::createCharExtensions(extensions, cExtensions);

                // Check mandatory features
                Vulkan::PhysicalDeviceFeatures mandatory;
                loadPhysicalDeviceFeatures(context, mandatory);

                // Build device
                ctxDevice = _manager.createRenderingDevice(cExtensions, mandatory, *_manager.getSurfaces().at(idSurface));
                // Register
                _devices[id] = ctxDevice; 

                // Load all device's data (order is important)
                loadSurfaces(context, ctxDevice);
            }
            else
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"DeviceModeError", context.getFileName(), mode);
            }
        }
        else
        {
            ctxDevice = _devices[id];
        }
        MOUCA_ASSERT(!ctxDevice.expired()); // Bad creation/ bad transfer ?

        // Load all device's data (order is important)
        loadShaderModules(context, ctxDevice);

        loadSemaphores(context, ctxDevice);

        loadFences(context, ctxDevice);

        loadImagesAndView(context, ctxDevice);

        loadSamplers(context, ctxDevice);

        loadBuffers(context, ctxDevice);

        loadRenderPasses(context, ctxDevice);

        loadFrameBuffers(context, ctxDevice);

        loadAccelerationStructures(context, ctxDevice);

        loadDescriptorSetPools(context, ctxDevice);

        loadDescriptorSetLayouts(context, ctxDevice);

        loadDescriptorSets(context, ctxDevice);

        loadPipelineLayouts(context, ctxDevice);

        loadGraphicsPipelines(context, ctxDevice);

        loadRayTracingPipelines(context, ctxDevice);

        loadTracingRay(context, ctxDevice);

        loadCommandPools(context, ctxDevice);

        loadCommandsGroup(context, ctxDevice);

        loadCommandBuffers(context, ctxDevice);

        loadQueueSequences(context, ctxDevice);
    }
}

}