/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTTypeInfo.h>
#include <LibVulkan/include/VKRenderer.h>

#include <MouCa3DEngine/interface/ISceneSynchronizer.h>

namespace RT
{
    class Resource;
    using ResourceWPtr = std::weak_ptr<Resource>;

    class Image;
    using ImageWPtr = std::weak_ptr<Image>;

    class Light;
    using LightWPtr = std::weak_ptr<Light>;
    
    class Origin;
}

namespace Vulkan
{
    class DeviceContext;

    class Renderable;
    using RenderableSPtr = std::shared_ptr<Renderable>;

    class Renderer;
    using RendererSPtr = std::shared_ptr<Renderer>;

    class Image;
    using ImageSPtr = std::shared_ptr<Image>;

    class LightBuffer;
    using LightBufferSPtr = std::shared_ptr<LightBuffer>;
    using LightBufferWPtr = std::weak_ptr<LightBuffer>;
}

namespace MouCaGraphic
{
    class GenericBuilder;
    using GenericBuilderUPtr = std::unique_ptr<GenericBuilder>;

    class RenderingSystem;

    //----------------------------------------------------------------------------
    /// \brief Allow to synchronize scene and make rendering object based on camera position.
    /// \code{.cpp}
    ///     
    /// \endcode
    class SceneSynchronizer : public ISceneSynchronizer
    {
        public:
            struct ImageComparison
            {
                bool operator() (const RT::ImageWPtr& lhs, const RT::ImageWPtr& rhs) const
                {
                    return lhs.lock() < rhs.lock();
                }
            };
            using ImageDictionnary = std::map<RT::ImageWPtr, Vulkan::ImageOldSPtr, ImageComparison>;

            /// Default constructor: make nothing.
            SceneSynchronizer(RenderingSystem& manager, Vulkan::Renderer& renderer);

            /// Destructor: check all releases have been called before delete itself.
            ~SceneSynchronizer() override;

            void initialize();

            void release();

            void registerBuilder(GenericBuilderUPtr&& builder);

            //------------------------------------------------------------------------
            /// \brief  Create/Update/Release scene objects based on mode and memory state.
            /// 
            void synchronizeScene(const RT::Scene& scene, const Mode mode) override;

            //------------------------------------------------------------------------
            /// \brief  Create/Update specific object to synchronize with renderer.
            /// 
            /// \param[in] objects: list of object to synchronize.
            void synchronize(const RT::Objects& objects) override;

            //------------------------------------------------------------------------
            /// \brief  Release specific object to synchronize with renderer.
            /// 
            /// \param[in] objects: list of object to synchronize.
            void release(const RT::Objects& objects) override;

            //------------------------------------------------------------------------
            /// \brief  Get LightBuffer to shared with renderer or other.
            /// 
            /// \returns LightBuffer which is build BUT no guaranty to be sync.
            Vulkan::LightBufferWPtr getLightBuffer() const
            {
                return _lightBuffer;
            }

            void setState( const State state ) override
            {
                _state = state;
            }
            
        protected:
            //------------------------------------------------------------------------
            /// \brief  Main synchronize method to compute object dictionary.
            /// 
            /// \param[in] object: object to synchronize.
            void synchronize( const RT::ObjectSPtr& object );

            struct ObjectComparison
            {
                bool operator() (const RT::ObjectWPtr& lhs, const RT::ObjectWPtr& rhs) const
                {
                    return lhs.lock() < rhs.lock();
                }
            };
            using Builders = std::map<RT::TypeInfo::Type, GenericBuilderUPtr>;
            using SceneDictionnary = std::map<RT::ObjectWPtr, std::vector<Vulkan::RenderableSPtr>, ObjectComparison>;
            
            RenderingSystem&        _manager;
            Vulkan::Renderer&       _renderer;

            Builders                _builders;
            SceneDictionnary        _dictionary;   ///< [LINK-LINK] Scene object dictionary of synchronization.
            ImageDictionnary        _images;       ///< Image dictionary of synchronization.
            
            Vulkan::LightBufferSPtr _lightBuffer;  ///< Light buffer (all scene lights)

            State                   _state;
            
    };

    using SceneSynchronizerSPtr = std::shared_ptr<SceneSynchronizer>;
}