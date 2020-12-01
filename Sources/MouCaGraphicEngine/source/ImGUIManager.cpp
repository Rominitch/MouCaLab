#include "Dependencies.h"

#include "MouCaGraphicEngine/include/ImGUIManager.h"

#include <LibVulkan/include/VKBuffer.h>
#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKContextDevice.h>

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTVirtualMouse.h>

namespace MouCaGraphic
{

RT::BufferDescriptor ImGUIManager::getMeshDescriptor()
{
    const std::vector<RT::ComponentDescriptor> descriptors =
    {
        { 2, RT::Type::Float, RT::ComponentUsage::Position  },
        { 2, RT::Type::Float, RT::ComponentUsage::TexCoord0 },
        { 4, RT::Type::UInt8, RT::ComponentUsage::Color,    true },
    };
    RT::BufferDescriptor bufferDescriptor;
    bufferDescriptor.initialize(descriptors);
    return bufferDescriptor;
}

ImGUIManager::ImGUIManager()
{
    MOUCA_PRE_CONDITION(isNull());
}

ImGUIManager::~ImGUIManager()
{
    MOUCA_PRE_CONDITION(isNull());
}

void ImGUIManager::initialize( const RT::Array2ui& resolution )
{
    MOUCA_PRE_CONDITION(isNull());

    // Start ImGUI
    ImGui::CreateContext();
    // Setup style
    ImGui::StyleColorsDark();

    _guiScale = 1.0f;

    // Color scheme
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg]          = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header]           = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button]           = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

    // Dimensions
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(resolution.x), static_cast<float>(resolution.y));
    io.FontGlobalScale = _guiScale;

    // Read font texture
    unsigned char* fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    _fontBuffer = std::make_shared<RT::BufferLinkedCPU>();
    _fontBuffer->create(RT::BufferDescriptor({ 4, RT::Type::UnsignedChar, RT::ComponentUsage::Color }), texWidth * texHeight, fontData);

    // Build font image
    auto font = std::make_shared<RT::ImageLinkedCPU>();
    font->initialize(RT::Image::Target::Type2D, _fontBuffer, {texWidth, texHeight, 1});
    _font = font;

    // Vulkan buffer
    _vertexBuffer = std::make_shared<Vulkan::Buffer>();
    _indexBuffer  = std::make_shared<Vulkan::Buffer>();

    _pushConstBuffer = std::make_shared<RT::BufferLinkedCPU>();
    _pushConstBuffer->create(RT::BufferDescriptor(sizeof(PushConstBlock)), 1, &_pushConstBlock);

    _pushConstBlock.scale     = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    _pushConstBlock.translate = glm::vec2(-1.0f);

    MOUCA_POST_CONDITION(!isNull());
}

void ImGUIManager::release(const Vulkan::ContextDevice& context)
{
    MOUCA_PRE_CONDITION(!isNull());

    const auto& device = context.getDevice();

    if (!_vertexBuffer->isNull())
    {
        _vertexBuffer->getMemory().unmap(device);
        _vertexBuffer->release(device);
    }

    if (!_indexBuffer->isNull())
    {
        _indexBuffer->getMemory().unmap(device);
        _indexBuffer->release(device);
    }

    _font->release();
    _font.reset();

    _fontBuffer->release();
    _fontBuffer.reset();

    ImGui::DestroyContext();

    MOUCA_POST_CONDITION(isNull());
}

/** Update vertex and index buffer containing the imGui elements when required */
bool ImGUIManager::prepareBuffer(const Vulkan::ContextDevice& context)
{
    MOUCA_PRE_CONDITION(!isNull());                        // DEV Issue: Missing to call initialize() ?
    MOUCA_PRE_CONDITION(!context.isNull());                // DEV Issue: Bad context ?
    MOUCA_PRE_CONDITION(ImGui::GetDrawData() != nullptr);  // DEV Issue: Missing to call initialize() ?

    ImDrawData* imDrawData = ImGui::GetDrawData();

    const auto& device = context.getDevice();

    bool updateCmdBuffers = false;

    // Note: Alignment is done inside buffer creation
    const VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    const VkDeviceSize indexBufferSize  = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    // Update buffers only if vertex or index count has been changed compared to current buffer size
    if ((vertexBufferSize == 0) || (indexBufferSize == 0))
    {
        return false;
    }

    // Vertex buffer
    if (_vertexBuffer->getSize() < vertexBufferSize)
    {
        if (!_vertexBuffer->isNull())
        {
            _vertexBuffer->getMemory().unmap(device);
            _vertexBuffer->release(device);
        }

        // Allocate new buffer more bigger
        _vertexBuffer->initialize(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
        _vertexBuffer->getMemory().map(device);
        updateCmdBuffers = true;
    }

    // Index buffer
    if (_indexBuffer->getSize() < indexBufferSize)
    {
        if (!_indexBuffer->isNull())
        {
            _indexBuffer->getMemory().unmap(device);
            _indexBuffer->release(device);
        }

        _indexBuffer->initialize(context.getDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize);
        _indexBuffer->getMemory().map(device);
        updateCmdBuffers = true;
    }

    // Upload data
    auto vtxDst = _vertexBuffer->getMemory().getMappedMemory<ImDrawVert>();
    auto idxDst = _indexBuffer->getMemory().getMappedMemory<ImDrawIdx>();

    for (int n = 0; n < imDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += cmd_list->VtxBuffer.Size;
        idxDst += cmd_list->IdxBuffer.Size;
    }

    // Flush to make writes visible to GPU
    _vertexBuffer->getMemory().flush(device);
    _indexBuffer->getMemory().flush(device);

    return updateCmdBuffers;
}

void ImGUIManager::buildCommands(Vulkan::Commands& commands)
{
    // Render commands
    /*
    command.emplace_back(_cmdDescriptor.get());
    command.emplace_back(_cmdPipeline.get());
    command.emplace_back(_cmdBind.get());
    command.emplace_back(_cmdPushConstant.get());
    */

    commands.clear();

    ImDrawData* imDrawData = ImGui::GetDrawData();
    MOUCA_ASSERT(imDrawData != nullptr);

    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;
    for (int32_t j = 0; j < imDrawData->CmdListsCount; j++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[j];
        for (int32_t k = 0; k < cmd_list->CmdBuffer.Size; k++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[k];
            const VkRect2D scissorRect
            {
                {
                    std::max(static_cast<int32_t>(pcmd->ClipRect.x), 0),
                    std::max(static_cast<int32_t>(pcmd->ClipRect.y), 0)
                },
                {
                    static_cast<uint32_t>(pcmd->ClipRect.z - pcmd->ClipRect.x),
                    static_cast<uint32_t>(pcmd->ClipRect.w - pcmd->ClipRect.y)
                }
            };
            
            commands.emplace_back(std::make_unique<Vulkan::CommandScissor>(scissorRect));
            commands.emplace_back(std::make_unique<Vulkan::CommandDrawIndexed>(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0));
            
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

void ImGUIManager::resize(RT::Window*, const RT::Array2ui&)
{}

void ImGUIManager::updateMouse(const RT::VirtualMouse& mouse) const
{
    ImGuiIO& io = ImGui::GetIO();

    const auto& posPx = mouse.getScreenPxPosition();
    io.MousePos = ImVec2(static_cast<float>(posPx.x), static_cast<float>(posPx.y));
    io.MouseDown[0] = mouse.isPressed(RT::VirtualMouse::Buttons::Left);
    io.MouseDown[1] = mouse.isPressed(RT::VirtualMouse::Buttons::Right);
    io.MouseDown[2] = mouse.isPressed(RT::VirtualMouse::Buttons::Middle);
}

}