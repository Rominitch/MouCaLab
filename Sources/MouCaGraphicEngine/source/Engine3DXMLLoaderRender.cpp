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

    auto result = context._parser.getNode(u8"RenderPasses");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto aPushR = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allRenderPasses = context._parser.getNode(u8"RenderPass");
        for (size_t idRenderPass = 0; idRenderPass < allRenderPasses->getNbElements(); ++idRenderPass)
        {
            auto renderPassNode = allRenderPasses->getNode(idRenderPass);
            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(renderPassNode, u8"RenderPass", _renderPasses, context, existing);
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
    auto allAttachments = context._parser.getNode(u8"Attachment");

    attachments.resize(allAttachments->getNbElements());
    for (size_t idAttachment = 0; idAttachment < allAttachments->getNbElements(); ++idAttachment)
    {
        auto attachmentNode = allAttachments->getNode(idAttachment);

        auto& attachment = attachments[idAttachment];
        // Extract VKImage format (from swapchain image or image)
        if (attachmentNode->hasAttribute(u8"surfaceId"))
        {
            const uint32_t surfaceId = LoaderHelper::getLinkedIdentifiant(attachmentNode, u8"surfaceId", _surfaces, context);

            // Use format of swapchain
            attachment.format = _surfaces[surfaceId].lock()->getFormat().getConfiguration()._format.format;
        }
        else if (attachmentNode->hasAttribute(u8"imageId"))
        {
            const uint32_t imageId = LoaderHelper::getLinkedIdentifiant(attachmentNode, u8"imageId", _images, context);

            // Use format of image
            attachment.format = _images[imageId].lock()->getFormat();
        }
        else
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassFormatError", context.getFileName(), std::to_string(id));
        }
        MouCa::assertion(attachment.format != VK_FORMAT_UNDEFINED);

        // Read sampler
        attachment.samples = LoaderHelper::readValue(attachmentNode, u8"samples", samples, false, context);
        MouCa::assertion(Core::Maths::isPowerOfTwo(static_cast<uint32_t>(attachment.samples)));

        // Read load/store
        attachment.loadOp = LoaderHelper::readValue(attachmentNode, u8"loadOp", attachmentLoads, false, context);
        attachment.storeOp = LoaderHelper::readValue(attachmentNode, u8"storeOp", attachmentStores, false, context);
        attachment.stencilLoadOp = LoaderHelper::readValue(attachmentNode, u8"stencilLoadOp", attachmentLoads, false, context);
        attachment.stencilStoreOp = LoaderHelper::readValue(attachmentNode, u8"stencilSaveOp", attachmentStores, false, context);

        // Read layout
        attachment.initialLayout = LoaderHelper::readValue(attachmentNode, u8"initialLayout", imageLayouts, false, context);
        attachment.finalLayout = LoaderHelper::readValue(attachmentNode, u8"finalLayout", imageLayouts, false, context);
    }

    // Manage errors
    if (attachments.empty())
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassMissAttachmentError", context.getFileName(), std::to_string(id));
    }
}

void Engine3DXMLLoader::loadRenderPassSubPass(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDescription>& subPasses, std::vector<SubPassAttachment>& attachmentReferences)
{
    auto allSubPasses = context._parser.getNode(u8"SubPass");

    // Allocated memory
    subPasses.resize(allSubPasses->getNbElements());
    attachmentReferences.resize(allSubPasses->getNbElements());

    for (size_t idSubPass = 0; idSubPass < allSubPasses->getNbElements(); ++idSubPass)
    {
        auto subPassNode = allSubPasses->getNode(idSubPass);
        auto& subPass = subPasses[idSubPass];
        auto& attachments = attachmentReferences[idSubPass];

        // Read data
        subPass.pipelineBindPoint = LoaderHelper::readValue(subPassNode, u8"bindPoint", bindPoints, false, context);

        auto aPush = context._parser.autoPushNode(*subPassNode);

        // Read color attachment
        auto allColorAttachments = context._parser.getNode(u8"ColorAttachment");
        attachments.buildColorAttachments(allColorAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allColorAttachments->getNbElements(); ++idAttachment)
        {
            auto colorAttachmentNode = allColorAttachments->getNode(idAttachment);

            // Read color Attachment
            attachments.colorAttachmentReferences[idAttachment] =
            {
                LoaderHelper::readValue(colorAttachmentNode, u8"colorAttachment", subPassAttachmentHelper, false, context),
                LoaderHelper::readValue(colorAttachmentNode, u8"colorLayout", imageLayouts, false, context)
            };

            // Read depth/stencil Attachment
            if (colorAttachmentNode->hasAttribute(u8"depthStencilAttachment"))
            {
                attachments.buildDepthAttachments();

                attachments.depthAttachmentReferences[idAttachment] =
                {
                    LoaderHelper::readValue(colorAttachmentNode, u8"depthStencilAttachment", subPassAttachmentHelper, false, context),
                    LoaderHelper::readValue(colorAttachmentNode, u8"depthStencilLayout", imageLayouts, false, context)
                };
            }
            // Read resolve Attachment (optional)
            if (colorAttachmentNode->hasAttribute(u8"resolveAttachment"))
            {
                // Allocate now buffer (re-entrance do nothing)
                attachments.buildResolveAttachments();

                attachments.colorAttachmentReferences[idAttachment] =
                {
                    LoaderHelper::readValue(colorAttachmentNode, u8"resolveAttachment", subPassAttachmentHelper, false, context),
                    LoaderHelper::readValue(colorAttachmentNode, u8"resolveLayout", imageLayouts, false, context)
                };
            }
        }

        if (attachments.colorAttachmentReferences.empty())
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassMissColorAttachmentError", context.getFileName(), std::to_string(idSubPass));
        }

        // Read color attachment
        auto allInputAttachments = context._parser.getNode(u8"InputAttachment");
        attachments.inputAttachmentReferences.resize(allInputAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allInputAttachments->getNbElements(); ++idAttachment)
        {
            auto inputAttachmentNode = allInputAttachments->getNode(idAttachment);

            // Read depth/stencil Attachment
            attachments.inputAttachmentReferences[idAttachment] =
            {
                LoaderHelper::readValue(inputAttachmentNode, u8"attachment", subPassAttachmentHelper, false, context),
                LoaderHelper::readValue(inputAttachmentNode, u8"layout",     imageLayouts, false, context)
            };
        }

        // Read preserve attachment
        auto allPreserveAttachments = context._parser.getNode(u8"PreserveAttachment");
        attachments.preserveAttachmentReferences.resize(allPreserveAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allPreserveAttachments->getNbElements(); ++idAttachment)
        {
            auto preserveAttachmentNode = allPreserveAttachments->getNode(idAttachment);
            attachments.preserveAttachmentReferences[idAttachment] = LoaderHelper::readValue(preserveAttachmentNode, u8"attachment", subPassAttachmentHelper, false, context);
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
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassMissSubpassError", context.getFileName(), std::to_string(id));
    }
}

void Engine3DXMLLoader::loadRenderPassDependency(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDependency>& dependencies, const std::vector<VkSubpassDescription>& subPass)
{
    auto allDependencies = context._parser.getNode(u8"Dependency");

    dependencies.reserve(allDependencies->getNbElements());
    for (size_t idDependency = 0; idDependency < allDependencies->getNbElements(); ++idDependency)
    {
        auto dependencyNode = allDependencies->getNode(idDependency);
        const VkSubpassDependency subDependency
        {
            LoaderHelper::readValue(dependencyNode, u8"srcSubpass",      subPassHelper,      false, context),
            LoaderHelper::readValue(dependencyNode, u8"dstSubpass",      subPassHelper,      false, context),
            LoaderHelper::readValue(dependencyNode, u8"srcStageMask",    pipelineStageFlags, true,  context),
            LoaderHelper::readValue(dependencyNode, u8"dstStageMask",    pipelineStageFlags, true,  context),
            LoaderHelper::readValue(dependencyNode, u8"srcAccessMask",   accessFlags,        true,  context),
            LoaderHelper::readValue(dependencyNode, u8"dstAccessMask",   accessFlags,        true,  context),
            LoaderHelper::readValue(dependencyNode, u8"dependencyFlags", dependencyFlags,    true,  context),
        };

        if (subDependency.dstSubpass != VK_SUBPASS_EXTERNAL && subDependency.dstSubpass >= subPass.size())
        {
            MOUCA_THROW_ERROR_4(u8"Engine3D", u8"XMLRenderPassUnknownSubpassError", context.getFileName(), std::to_string(id), u8"dstSubpass", std::to_string(subDependency.dstSubpass));
        }
        if (subDependency.srcSubpass != VK_SUBPASS_EXTERNAL && subDependency.srcSubpass >= subPass.size())
        {
            MOUCA_THROW_ERROR_4(u8"Engine3D", u8"XMLRenderPassUnknownSubpassError", context.getFileName(), std::to_string(id), u8"srcSubpass", std::to_string(subDependency.srcSubpass));
        }
        dependencies.emplace_back(subDependency);
    }
}
}