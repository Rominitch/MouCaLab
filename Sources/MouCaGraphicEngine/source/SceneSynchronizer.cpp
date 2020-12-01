#include "Dependencies.h"
/*
#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTMassiveInstance.h>
#include <LibRT/include/RTScene.h>

#include <LibVulkan/include/VKDeviceContext.h>
#include <LibVulkan/include/VKLightBuffer.h>

#include <LibVulkan/include/VKRenderer.h>

// Builder
#include <MouCaGraphicEngine/include/SceneBuilder/GenericBuilder.h>

#include <MouCaGraphicEngine/include/Engine3DLoader.h>
#include <MouCaGraphicEngine/include/RenderPathWorld.h>
#include <MouCaGraphicEngine/include/Renderer/RendererManager.h>
#include <MouCaGraphicEngine/include/SceneSynchronizer.h>

namespace MouCa3DEngine
{

SceneSynchronizer::SceneSynchronizer(RenderingSystem& manager, Vulkan::Renderer& renderer):
_manager(manager), _renderer(renderer), _lightBuffer(std::make_shared<Vulkan::LightBuffer>()), _state(NeedBuildCommand)
{}

SceneSynchronizer::~SceneSynchronizer()
{
    MOUCA_PRE_CONDITION(_dictionary.empty());
    MOUCA_PRE_CONDITION(_images.empty());
    MOUCA_PRE_CONDITION(_lightBuffer.get() == nullptr);
}

void SceneSynchronizer::initialize()
{
    // Allocate light buffer
    _lightBuffer->initialize(_manager.getDeviceContext());
}

void SceneSynchronizer::release()
{
    _dictionary.clear();

    for(auto& image: _images)
    {
        image.second->release(_manager.getDeviceContext().getDevice());
    }
    _images.clear();

    // Allocate light buffer
    _lightBuffer->release(_manager.getDeviceContext());
    _lightBuffer.reset();
}

void SceneSynchronizer::registerBuilder(GenericBuilderUPtr&& builder)
{
    MOUCA_PRE_CONDITION(_builders.find(builder->getType()) == _builders.cend());
    _builders[builder->getType()] = std::move(builder);
}

void SceneSynchronizer::synchronizeScene(const RT::Scene& scene, const Mode)
{
    MOUCA_PRE_CONDITION(!_renderer.isNull());
    MOUCA_PRE_CONDITION(!_builders.empty());                       // DEV Issue: Nothing to build !
    
    _renderer.lockRendering();

    const Vulkan::DeviceContext& deviceContext = _manager.getDeviceContext();

    for(const auto& geometry : scene.getGeometries() )
    {
        synchronize( geometry );
    }
    for(const auto& massive : scene.getMassiveInstances())
    {
        synchronize(massive);
    }
    for(const auto& objects : scene.getObjects())
    {
        synchronize(objects);
    }
    
    // Update lighting
    Vulkan::LightBuffer::Lighting lighting;
    lighting._nbLights = static_cast<int>(scene.getLights().size());
    auto itLight = lighting._lights.begin();
    for(const auto& light : scene.getLights() )
    {
        *itLight = { glm::vec4(light->getOrientation().getPosition(), 1.0f), light->getColor(), light->getRadius() };
        ++itLight;
    }
    _lightBuffer->update(deviceContext, lighting);

    // Build command buffer
    if( _state == NeedBuildCommand )
        _renderer.buildCommandBuffers( deviceContext );

    _state = IsUpdated;
    _renderer.unlockRendering();
    MOUCA_POST_CONDITION( !_renderer.isNull() );
}

void SceneSynchronizer::synchronize( const RT::ObjectSPtr& object )
{
    MOUCA_PRE_CONDITION(!_renderer.isNull());

    // Search builder
    auto builder = _builders.find(object->getType());
    if( builder != _builders.cend() )
    {
        // Search if object exists into dict
        const auto itObject = _dictionary.find(object);
        
        // Need to build ?
        std::vector<Vulkan::RenderableSPtr> renderables;
        if( itObject == _dictionary.cend() )
        {
            // Build and save instance
            builder->second->build(_manager, object, renderables);

            _dictionary[object] = renderables;
        }
        else
        {
            renderables = itObject->second;
        }

        // Update object
        builder->second->update(_manager, object, renderables);
    }
    else
    {
       MOUCA_THROW_ERROR(u8"BasicError", u8"UnknownEnumError")
    }
}

void SceneSynchronizer::synchronize(const RT::Objects&)
{}

void SceneSynchronizer::release(const RT::Objects&)
{}

}
*/