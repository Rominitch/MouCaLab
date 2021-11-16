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
    MouCa::preCondition(context._parser.isLoaded());           //DEV Issue: Need a valid xml.
    MouCa::preCondition(_manager.getEnvironment().isNull());    //DEV Issue: Environment MUST be not configured.

    // Check node exists
    auto result = context._parser.getNode("Environment");
    if (result->getNbElements() == 0)
    {
        throw Core::Exception(makeLoaderError(context, "LoadingXMLCorruptError") << "Environment");
    }

    // Go into
    XML::NodeUPtr environmentNode = result->getNode(0);
    
    // Read App info
    environmentNode->getAttribute("application", context._info._applicationName);
    environmentNode->getAttribute("engine", context._info._engineName);

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

    MouCa::postCondition(!_manager.getEnvironment().isNull());  //DEV Issue: Environment MUST be configured.
}

void Engine3DXMLLoader::loadExtensions(ContextLoading& context, std::vector<Core::String>& extensions)
{
    // Read extensions
    auto result = context._parser.getNode("Extension");

    // Allocate more space
    extensions.reserve(extensions.size() + result->getNbElements());

    for (size_t idExtension = 0; idExtension < result->getNbElements(); ++idExtension)
    {
        Core::String osName;
        auto extension = result->getNode(idExtension);
        if (extension->hasAttribute("os"))
        {
            extension->getAttribute("os", osName);
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
    auto result = context._parser.getNode("FeatureBufferDeviceAddress");
    if(result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        featureNode->getAttribute("bufferDeviceAddress", enable);

        mandatory._bufferDeviceAddresFeatures.bufferDeviceAddress = toVKBool(enable);
    }

    result = context._parser.getNode("FeatureRayTracingPipeline");
    if (result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        featureNode->getAttribute("rayTracingPipeline", enable);

        mandatory._rayTracingPipelineFeatures.rayTracingPipeline = toVKBool(enable);
    }

    result = context._parser.getNode("FeatureAccelerationStructure");
    if (result->getNbElements() == 1)
    {
        auto featureNode = result->getNode(0);
        featureNode->getAttribute("accelerationStructure", enable);

        mandatory._accelerationStructureFeatures.accelerationStructure = toVKBool(enable);
    }

    result = context._parser.getNode("Features");
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

        enableFeature("alphaToOne",                               mandatory._features.alphaToOne);
        enableFeature("depthBiasClamp",                           mandatory._features.depthBiasClamp);
        enableFeature("depthBounds",                              mandatory._features.depthBounds);
        enableFeature("depthClamp",                               mandatory._features.depthClamp);
        enableFeature("drawIndirectFirstInstance",                mandatory._features.drawIndirectFirstInstance);
        enableFeature("dualSrcBlend",                             mandatory._features.dualSrcBlend);
        enableFeature("fillModeNonSolid",                         mandatory._features.fillModeNonSolid);
        enableFeature("fragmentStoresAndAtomics",                 mandatory._features.fragmentStoresAndAtomics);
        enableFeature("fullDrawIndexUint32",                      mandatory._features.fullDrawIndexUint32);
        enableFeature("geometryShader",                           mandatory._features.geometryShader);
        enableFeature("imageCubeArray",                           mandatory._features.imageCubeArray);
        enableFeature("independentBlend",                         mandatory._features.independentBlend);
        enableFeature("inheritedQueries",                         mandatory._features.inheritedQueries);
        enableFeature("largePoints",                              mandatory._features.largePoints);
        enableFeature("samplelogicOprAnisotropy",                 mandatory._features.logicOp);
        enableFeature("multiDrawIndirect",                        mandatory._features.multiDrawIndirect);
        enableFeature("multiViewport",                            mandatory._features.multiViewport);
        enableFeature("occlusionQueryPrecise",                    mandatory._features.occlusionQueryPrecise);
        enableFeature("pipelineStatisticsQuery",                  mandatory._features.pipelineStatisticsQuery);
        enableFeature("robustBufferAccess",                       mandatory._features.robustBufferAccess);
        enableFeature("samplerAnisotropy",                        mandatory._features.samplerAnisotropy);
        enableFeature("sampleRateShading",                        mandatory._features.sampleRateShading);
        enableFeature("shaderClipDistance",                       mandatory._features.shaderClipDistance);
        enableFeature("shaderCullDistance",                       mandatory._features.shaderCullDistance);
        enableFeature("shaderFloat64",                            mandatory._features.shaderFloat64);
        enableFeature("shaderImageGatherExtended",                mandatory._features.shaderImageGatherExtended);
        enableFeature("shaderInt16",                              mandatory._features.shaderInt16);
        enableFeature("shaderInt64",                              mandatory._features.shaderInt64);
        enableFeature("shaderResourceMinLod",                     mandatory._features.shaderResourceMinLod);
        enableFeature("shaderResourceResidency",                  mandatory._features.shaderResourceResidency);
        enableFeature("shaderSampledImageArrayDynamicIndexing",   mandatory._features.shaderSampledImageArrayDynamicIndexing);
        enableFeature("shaderStorageBufferArrayDynamicIndexing",  mandatory._features.shaderStorageBufferArrayDynamicIndexing);
        enableFeature("shaderStorageImageArrayDynamicIndexing",   mandatory._features.shaderStorageImageArrayDynamicIndexing);
        enableFeature("shaderStorageImageExtendedFormats",        mandatory._features.shaderStorageImageExtendedFormats);
        enableFeature("shaderStorageImageMultisample",            mandatory._features.shaderStorageImageMultisample);
        enableFeature("shaderStorageImageReadWithoutFormat",      mandatory._features.shaderStorageImageReadWithoutFormat);
        enableFeature("shaderStorageImageWriteWithoutFormat",     mandatory._features.shaderStorageImageWriteWithoutFormat);
        enableFeature("shaderTessellationAndGeometryPointSize",   mandatory._features.shaderTessellationAndGeometryPointSize);
        enableFeature("shaderUniformBufferArrayDynamicIndexing",  mandatory._features.shaderUniformBufferArrayDynamicIndexing);
        enableFeature("sparseBinding",                            mandatory._features.sparseBinding);
        enableFeature("sparseResidency16Samples",                 mandatory._features.sparseResidency16Samples);
        enableFeature("sparseResidency2Samples",                  mandatory._features.sparseResidency2Samples);
        enableFeature("sparseResidency4Samples",                  mandatory._features.sparseResidency4Samples);
        enableFeature("sparseResidency8Samples",                  mandatory._features.sparseResidency8Samples);
        enableFeature("sparseResidencyAliased",                   mandatory._features.sparseResidencyAliased);
        enableFeature("sparseResidencyBuffer",                    mandatory._features.sparseResidencyBuffer);
        enableFeature("sparseResidencyImage2D",                   mandatory._features.sparseResidencyImage2D);
        enableFeature("sparseResidencyImage3D",                   mandatory._features.sparseResidencyImage3D);
        enableFeature("tessellationShader",                       mandatory._features.tessellationShader);
        enableFeature("textureCompressionASTC_LDR",               mandatory._features.textureCompressionASTC_LDR);
        enableFeature("textureCompressionBC",                     mandatory._features.textureCompressionBC);
        enableFeature("textureCompressionETC2",                   mandatory._features.textureCompressionETC2);
        enableFeature("sparseBinding",                            mandatory._features.sparseBinding);
        enableFeature("variableMultisampleRate",                  mandatory._features.variableMultisampleRate);
        enableFeature("vertexPipelineStoresAndAtomics",           mandatory._features.vertexPipelineStoresAndAtomics);
        enableFeature("wideLines",                                mandatory._features.wideLines);
    }
}

void Engine3DXMLLoader::loadDevices(ContextLoading& context)
{
    MouCa::preCondition(context._parser.isLoaded()); //DEV Issue: Need a valid xml.

    auto result = context._parser.getNode("Device");
    for (size_t idDevice = 0; idDevice < result->getNbElements(); ++idDevice)
    {
        auto device = result->getNode(idDevice);

        Vulkan::ContextDeviceWPtr ctxDevice;
        bool existing = false;
        const uint32_t id = LoaderHelper::getIdentifiant(device, "Device", _devices, context, existing);
        if (!existing)
        {
            Core::String mode;
            device->getAttribute("mode", mode);
            if (mode == "render")
            {
                uint32_t idSurface = 0;
                device->getAttribute("compatibleWindowId", idSurface);

                if (idSurface >= _manager.getSurfaces().size())
                {
                    throw Core::Exception(makeLoaderError(context, "UnknownSurfaceError"));
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
                throw Core::Exception(makeLoaderError(context, "DeviceModeError") << mode);
            }
        }
        else
        {
            ctxDevice = _devices[id];
        }
        MouCa::assertion(!ctxDevice.expired()); // Bad creation/ bad transfer ?

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