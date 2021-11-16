#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DXMLLoader.h"
#include "MouCaGraphicEngine/include/Engine3DXMLHelper.h"

#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKImage.h>
#include <LibVulkan/include/VKRenderPass.h>
#include <LibVulkan/include/VKSampler.h>

#include "MouCaGraphicEngine/include/VulkanEnum.h"

namespace MouCaGraphic
{

struct SubPassAttachment
{
    void buildColorAttachments(size_t size)
    {
        colorAttachmentReferences.resize(size);
    }

    void buildAttachments(std::vector<VkAttachmentReference>& attachmentReferences)
    {
        MouCa::preCondition(!colorAttachmentReferences.empty());
        if (attachmentReferences.empty())
        {
            attachmentReferences.resize(colorAttachmentReferences.size());

            const VkAttachmentReference empty{ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
            std::fill(attachmentReferences.begin(), attachmentReferences.end(), empty);
        }
    }

    void buildDepthAttachments()
    {
        buildAttachments(depthAttachmentReferences);
    }

    void buildResolveAttachments()
    {
        buildAttachments(resolveAttachmentReferences);
    }

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkAttachmentReference> depthAttachmentReferences;
    std::vector<VkAttachmentReference> resolveAttachmentReferences;

    std::vector<VkAttachmentReference> inputAttachmentReferences;

    std::vector<uint32_t> preserveAttachmentReferences;
};

void Engine3DXMLLoader::loadRenderPasses(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    auto result = context._parser.getNode("RenderPasses");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto aPushR = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allRenderPasses = context._parser.getNode("RenderPass");
        for (size_t idRenderPass = 0; idRenderPass < allRenderPasses->getNbElements(); ++idRenderPass)
        {
            auto renderPassNode = allRenderPasses->getNode(idRenderPass);
            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(renderPassNode, "RenderPass", _renderPasses, context, existing);
            if (existing)
                continue;

            auto aPush = context._parser.autoPushNode(*renderPassNode);

            // Load data (WARNING: vector MUST be valid/allocated until renderPass->initialize)
            std::vector<VkAttachmentDescription> attachments;
            loadRenderPassAttachment(context, id, attachments);

            std::vector<SubPassAttachment> attachmentReferences;
            std::vector<VkSubpassDescription> subpasses;
            loadRenderPassSubPass(context, id, subpasses, attachmentReferences);

            std::vector<VkSubpassDependency> dependencies;
            loadRenderPassDependency(context, id, dependencies, subpasses);

            // Build render pass
            auto renderPass = std::make_shared<Vulkan::RenderPass>();
            renderPass->initialize(device->getDevice(), std::move(attachments), std::move(subpasses), std::move(dependencies));

            // Register + ownership
            device->insertRenderPass(renderPass);

            _renderPasses[id] = renderPass;
        }
    }
}

void Engine3DXMLLoader::loadRenderPassAttachment(ContextLoading& context, const uint32_t id, std::vector<VkAttachmentDescription>& attachments)
{
    auto allAttachments = context._parser.getNode("Attachment");

    attachments.resize(allAttachments->getNbElements());
    for (size_t idAttachment = 0; idAttachment < allAttachments->getNbElements(); ++idAttachment)
    {
        auto attachmentNode = allAttachments->getNode(idAttachment);

        auto& attachment = attachments[idAttachment];
        // Extract VKImage format (from swapchain image or image)
        if (attachmentNode->hasAttribute("surfaceId"))
        {
            const uint32_t surfaceId = LoaderHelper::getLinkedIdentifiant(attachmentNode, "surfaceId", _surfaces, context);

            // Use format of swapchain
            attachment.format = _surfaces[surfaceId].lock()->getFormat().getConfiguration()._format.format;
        }
        else if (attachmentNode->hasAttribute("imageId"))
        {
            const uint32_t imageId = LoaderHelper::getLinkedIdentifiant(attachmentNode, "imageId", _images, context);

            // Use format of image
            attachment.format = _images[imageId].lock()->getFormat();
        }
        else
        {
            throw Core::Exception(Core::ErrorData("Engine3D", "XMLRenderPassFormatError") << context.getFileName() << std::to_string(id));
        }
        MouCa::assertion(attachment.format != VK_FORMAT_UNDEFINED);

        // Read sampler
        attachment.samples = LoaderHelper::readValue(attachmentNode, "samples", samples, false, context);
        MouCa::assertion(Core::Maths::isPowerOfTwo(static_cast<uint32_t>(attachment.samples)));

        // Read load/store
        attachment.loadOp = LoaderHelper::readValue(attachmentNode, "loadOp", attachmentLoads, false, context);
        attachment.storeOp = LoaderHelper::readValue(attachmentNode, "storeOp", attachmentStores, false, context);
        attachment.stencilLoadOp = LoaderHelper::readValue(attachmentNode, "stencilLoadOp", attachmentLoads, false, context);
        attachment.stencilStoreOp = LoaderHelper::readValue(attachmentNode, "stencilSaveOp", attachmentStores, false, context);

        // Read layout
        attachment.initialLayout = LoaderHelper::readValue(attachmentNode, "initialLayout", imageLayouts, false, context);
        attachment.finalLayout = LoaderHelper::readValue(attachmentNode, "finalLayout", imageLayouts, false, context);
    }

    // Manage errors
    if (attachments.empty())
    {
        throw Core::Exception(Core::ErrorData("Engine3D", "XMLRenderPassMissAttachmentError") << context.getFileName() << std::to_string(id));
    }
}

void Engine3DXMLLoader::loadRenderPassSubPass(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDescription>& subPasses, std::vector<SubPassAttachment>& attachmentReferences)
{
    auto allSubPasses = context._parser.getNode("SubPass");

    // Allocated memory
    subPasses.resize(allSubPasses->getNbElements());
    attachmentReferences.resize(allSubPasses->getNbElements());

    for (size_t idSubPass = 0; idSubPass < allSubPasses->getNbElements(); ++idSubPass)
    {
        auto subPassNode = allSubPasses->getNode(idSubPass);
        auto& subPass = subPasses[idSubPass];
        auto& attachments = attachmentReferences[idSubPass];

        // Read data
        subPass.pipelineBindPoint = LoaderHelper::readValue(subPassNode, "bindPoint", bindPoints, false, context);

        auto aPush = context._parser.autoPushNode(*subPassNode);

        // Read color attachment
        auto allColorAttachments = context._parser.getNode("ColorAttachment");
        attachments.buildColorAttachments(allColorAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allColorAttachments->getNbElements(); ++idAttachment)
        {
            auto colorAttachmentNode = allColorAttachments->getNode(idAttachment);

            // Read color Attachment
            attachments.colorAttachmentReferences[idAttachment] =
            {
                LoaderHelper::readValue(colorAttachmentNode, "colorAttachment", subPassAttachmentHelper, false, context),
                LoaderHelper::readValue(colorAttachmentNode, "colorLayout", imageLayouts, false, context)
            };

            // Read depth/stencil Attachment
            if (colorAttachmentNode->hasAttribute("depthStencilAttachment"))
            {
                attachments.buildDepthAttachments();

                attachments.depthAttachmentReferences[idAttachment] =
                {
                    LoaderHelper::readValue(colorAttachmentNode, "depthStencilAttachment", subPassAttachmentHelper, false, context),
                    LoaderHelper::readValue(colorAttachmentNode, "depthStencilLayout", imageLayouts, false, context)
                };
            }
            // Read resolve Attachment (optional)
            if (colorAttachmentNode->hasAttribute("resolveAttachment"))
            {
                // Allocate now buffer (re-entrance do nothing)
                attachments.buildResolveAttachments();

                attachments.colorAttachmentReferences[idAttachment] =
                {
                    LoaderHelper::readValue(colorAttachmentNode, "resolveAttachment", subPassAttachmentHelper, false, context),
                    LoaderHelper::readValue(colorAttachmentNode, "resolveLayout", imageLayouts, false, context)
                };
            }
        }

        if (attachments.colorAttachmentReferences.empty())
        {
            throw Core::Exception(Core::ErrorData("Engine3D", "XMLRenderPassMissColorAttachmentError") << context.getFileName() << std::to_string(idSubPass));
        }

        // Read color attachment
        auto allInputAttachments = context._parser.getNode("InputAttachment");
        attachments.inputAttachmentReferences.resize(allInputAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allInputAttachments->getNbElements(); ++idAttachment)
        {
            auto inputAttachmentNode = allInputAttachments->getNode(idAttachment);

            // Read depth/stencil Attachment
            attachments.inputAttachmentReferences[idAttachment] =
            {
                LoaderHelper::readValue(inputAttachmentNode, "attachment", subPassAttachmentHelper, false, context),
                LoaderHelper::readValue(inputAttachmentNode, "layout",     imageLayouts, false, context)
            };
        }

        // Read preserve attachment
        auto allPreserveAttachments = context._parser.getNode("PreserveAttachment");
        attachments.preserveAttachmentReferences.resize(allPreserveAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allPreserveAttachments->getNbElements(); ++idAttachment)
        {
            auto preserveAttachmentNode = allPreserveAttachments->getNode(idAttachment);
            attachments.preserveAttachmentReferences[idAttachment] = LoaderHelper::readValue(preserveAttachmentNode, "attachment", subPassAttachmentHelper, false, context);
        }

        // Link array to data
        subPass.colorAttachmentCount = static_cast<uint32_t>(attachments.colorAttachmentReferences.size());
        subPass.pColorAttachments = attachments.colorAttachmentReferences.data();
        subPass.pDepthStencilAttachment = attachments.depthAttachmentReferences.empty()
            ? VK_NULL_HANDLE
            : attachments.depthAttachmentReferences.data();
        subPass.pResolveAttachments = attachments.resolveAttachmentReferences.empty()
            ? VK_NULL_HANDLE
            : attachments.resolveAttachmentReferences.data();

        subPass.inputAttachmentCount = static_cast<uint32_t>(attachments.inputAttachmentReferences.size());
        subPass.pInputAttachments = attachments.inputAttachmentReferences.data();

        subPass.preserveAttachmentCount = static_cast<uint32_t>(attachments.preserveAttachmentReferences.size());
        subPass.pPreserveAttachments = attachments.preserveAttachmentReferences.data();
    }

    if (subPasses.empty())
    {
        throw Core::Exception(Core::ErrorData("Engine3D", "XMLRenderPassMissSubpassError") << context.getFileName() << std::to_string(id));
    }
}

void Engine3DXMLLoader::loadRenderPassDependency(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDependency>& dependencies, const std::vector<VkSubpassDescription>& subPass)
{
    auto allDependencies = context._parser.getNode("Dependency");

    dependencies.reserve(allDependencies->getNbElements());
    for (size_t idDependency = 0; idDependency < allDependencies->getNbElements(); ++idDependency)
    {
        auto dependencyNode = allDependencies->getNode(idDependency);
        const VkSubpassDependency subDependency
        {
            LoaderHelper::readValue(dependencyNode, "srcSubpass",      subPassHelper,      false, context),
            LoaderHelper::readValue(dependencyNode, "dstSubpass",      subPassHelper,      false, context),
            LoaderHelper::readValue(dependencyNode, "srcStageMask",    pipelineStageFlags, true,  context),
            LoaderHelper::readValue(dependencyNode, "dstStageMask",    pipelineStageFlags, true,  context),
            LoaderHelper::readValue(dependencyNode, "srcAccessMask",   accessFlags,        true,  context),
            LoaderHelper::readValue(dependencyNode, "dstAccessMask",   accessFlags,        true,  context),
            LoaderHelper::readValue(dependencyNode, "dependencyFlags", dependencyFlags,    true,  context),
        };

        if (subDependency.dstSubpass != VK_SUBPASS_EXTERNAL && subDependency.dstSubpass >= subPass.size())
        {
            throw Core::Exception(Core::ErrorData("Engine3D", "XMLRenderPassUnknownSubpassError") << context.getFileName() << std::to_string(id) << "dstSubpass" << std::to_string(subDependency.dstSubpass));
        }
        if (subDependency.srcSubpass != VK_SUBPASS_EXTERNAL && subDependency.srcSubpass >= subPass.size())
        {
            throw Core::Exception(Core::ErrorData("Engine3D", "XMLRenderPassUnknownSubpassError") << context.getFileName() << std::to_string(id) << "srcSubpass" << std::to_string(subDependency.srcSubpass));
        }
        dependencies.emplace_back(subDependency);
    }
}
}