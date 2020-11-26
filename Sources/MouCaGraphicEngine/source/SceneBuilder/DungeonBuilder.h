#include "Dependancies.h"

#include <LibVulkan/include/VKDeviceContext.h>
#include <LibVulkan/include/VKRenderer.h>

#include <MouCaGameEngine/interface/IDungeon.h>

#include "MouCa3DVulkanEngine/include/Engine3DLoader.h"
#include "MouCa3DVulkanEngine/include/RendererManager.h"
#include "MouCa3DVulkanEngine/include/RenderPathWorld.h" // To Split and remove

#include "MouCa3DVulkanEngine/include/SceneBuilder/MapWorldTerrainFlatBuilder.h"

void MapWorldTerrainFlatBuilder::build(RenderingSystem& manager, const RT::ObjectSPtr& model, std::vector<Vulkan::RenderableSPtr>& renderables) const
{
    const Vulkan::DeviceContext& deviceContext = manager.getDeviceContext();

    BT_PRE_CONDITION(!deviceContext.getDevice().isNull());
    BT_PRE_CONDITION(!deviceContext.getCommandPool().isNull());

    BT_PRE_CONDITION(dynamic_cast<MouCaGame::IDungeon*>( model.get() ) != nullptr);
    MouCaGame::IDungeon* dungeon = static_cast<MouCaGame::IDungeon*>( model.get() );

    // Creation
    Vulkan::RenderableSPtr level0;
    dungeon.

    // Register scene
    renderables = { level0 };

    // Register GPU
    BT_ASSERT(dynamic_cast<Vulkan::Renderer*>( manager.getFirstRenderer().get() ) != nullptr);
    static_cast<Vulkan::Renderer*>( manager.getFirstRenderer().get() )->addRenderable(deviceContext, level0);
}

void MapWorldTerrainFlatBuilder::update(RenderingSystem& manager, const RT::ObjectSPtr& model, std::vector<Vulkan::RenderableSPtr>& renderables) const
{}
