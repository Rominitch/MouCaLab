#pragma once

#include <MouCaCore/include/Core.h>

#include <LibCore/include/CoreFileTracker.h>

namespace MouCaCore
{
    class ResourceManager final : public IResourceManager, public std::enable_shared_from_this<ResourceManager>
    {
        MOUCA_NOCOPY_NOMOVE(ResourceManager);

        public:
            enum ResourceFolder
            {
                //System -> from 0x01 to 0xff
                Executable      = 0x00000001,
                Error           = 0x00000002,

                //Inputs -> from 0x0100 to 0xff00
                Configuration   = 0x00000100,
                Meshes          = 0x00000200,
                Shaders         = 0x00000300,
                References      = 0x00000400,
                Textures        = 0x00000500,
                Fonts           = 0x00000600,

                //Outputs -> from 0x010000 to 0xff0000
                Generated       = 0x00010000,

                //Admin -> from 0x01000000 to 0xff000000
                AdminGenerated  = 0x01000000,
                ShadersSource   = 0x02000000,
                Renderer        = 0x03000000
            };

            ResourceManager() = default;

            ~ResourceManager() override = default;

        //--------------------------------------------------------------------------
        //								Folder management
        //--------------------------------------------------------------------------
            void addResourceFolder(const Core::Path& folder, const size_t idFolder);

            const Core::Path& getResourceFolder(const size_t idFolder) const
            {
                MOUCA_ASSERT(_mapFolders.find(idFolder) != _mapFolders.cend()); // DEV Issue: never instantiate folder ID !
                return _mapFolders.at(idFolder);
            }

        //--------------------------------------------------------------------------
        //							   Resources management
        //--------------------------------------------------------------------------
            void registerResource(Core::ResourceSPtr resource);

            void releaseResource(Core::ResourceSPtr&& resource) override;

            void releaseResources() override;

            void unregisterResource(Core::Resource* resource)
            {
                auto itResource = std::find_if(_resources.begin(), _resources.cend(), [&](const Core::ResourceSPtr& item){ return resource == item.get(); });
                MOUCA_PRE_CONDITION(itResource != _resources.cend());
                _resources.erase(itResource);
            }

            size_t getNbResources() const
            {
                return _resources.size();
            }

            const std::set<Core::ResourceSPtr>& getResources() const
            {
                return _resources;
            }

            Core::FileTracker& getTracker()
            {
                return _tracker;
            }

        //--------------------------------------------------------------------------
        //							  Writable resources
        //--------------------------------------------------------------------------
            Core::FileSPtr createFile(const Core::Path& filename = Core::Path());

            XML::ParserSPtr createXML(const Core::Path& filename = Core::Path());

            RT::ImageImportSPtr createImage(const Core::Path& filename = Core::Path());

            DatabaseSPtr createDatabase();

        //--------------------------------------------------------------------------
        //						    Readable only resources
        //--------------------------------------------------------------------------
            RT::ShaderFileSPtr openShader( const Core::Path& filename, const RT::ShaderKind kind, const Core::StringOS& sourceShader = Core::StringOS() );

            Core::FileSPtr openFile(const Core::Path& filename);

            XML::ParserSPtr openXML(const Core::Path& filename);

            RT::ImageImportSPtr openImage(const Core::Path& filename);

            RT::MeshImportSPtr openMeshImport(const Core::Path& filename, const RT::BufferDescriptor& descriptor = RT::BufferDescriptor(), const RT::MeshImport::Flag flag = RT::MeshImport::ComputeAll);

            RT::AnimationImporterSPtr openAnimation(const Core::Path& filename);

        protected:
            template<typename ResourceType>
            Core::ResourceSPtr searchResources(const Core::Path& filename) const;

            template<typename BuildClass>
            std::shared_ptr<BuildClass> genericOpen( const Core::Path& filename );

            std::map<size_t, Core::Path>	_mapFolders;	///< Contains all inputs/outputs folders by category

            std::set<Core::ResourceSPtr>	_resources;	    ///< [OWNERSHIP] Contains all usable resources.
            Core::FileTracker               _tracker;       ///< [OWNERSHIP] Contains manager of tracking.
    };

    using ResourceManagerSPtr = std::shared_ptr<ResourceManager>;
}