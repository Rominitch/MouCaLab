/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTEnum.h>
#include <LibRT/include/RTMesh.h>

namespace Core
{
    class Exception;

    class Locale; 

    class PlugInManager;
    class ThreadPools;

    class IErrorManager;

    class Resource;
    using ResourceSPtr = std::shared_ptr<Resource>;

    class File;
    using FileSPtr = std::shared_ptr<File>;
    using FileWPtr = std::weak_ptr<File>;

    class FileTracker;

    using Path = std::filesystem::path;
}

namespace XML
{
    class Parser;
    using ParserSPtr = std::shared_ptr<Parser>;
}

namespace RT
{
    class ImageImport;
    using ImageImportSPtr = std::shared_ptr<ImageImport>;
    using ImageImportWPtr = std::weak_ptr<ImageImport>;

    class MeshImport;
    using MeshImportSPtr = std::shared_ptr<MeshImport>;

    class ShaderFile;
    using ShaderFileSPtr = std::shared_ptr<ShaderFile>;

    class BufferDescriptor;

    class AnimationImporter;
    using AnimationImporterSPtr = std::shared_ptr<AnimationImporter>;
}

namespace MouCaCore
{
    class Database;
    using DatabaseSPtr = std::shared_ptr<Database>;

    struct LoadingItem
    {
        enum State
        {
            Direct,         ///< Need to load data now.
            Deferred        ///< Need to load data soon.
        };

        enum Order
        {
            Urgent,
            Priority,
            Normal,
            Low,
            VeryLow,
            NbOrders
        };

        //------------------------------------------------------------------------
        /// \brief  Constructor
        /// 
        /// \param[in,out] resource:
        /// \param[in] state:
        /// \param[in] order:
        LoadingItem(Core::ResourceSPtr resource = Core::ResourceSPtr(), const State state = Direct, const Order order = Normal) :
            _resource(resource),
            _state(state),
            _order(order)
        {}

        /// Destructor
        ~LoadingItem() = default;

        Core::ResourceSPtr  _resource;      ///< Data pointer.
        State               _state;         ///< State of loading.
        Order               _order;         ///< Priority of loading.

        bool operator<(const LoadingItem& item) const
        {
            return _state < item._state || (_state == item._state && _order < item._order);
        }
    };
    using LoadingItems = std::deque<LoadingItem>;

    class ILoaderManager
    {
        MOUCA_NOCOPY_NOMOVE(ILoaderManager);
        public:
            virtual ~ILoaderManager() = default;

            virtual void initialize(const uint32_t nbQueue = 5) = 0;

            virtual void release() = 0;

            virtual void loadResources(LoadingItems& queue) = 0;

            virtual void synchronize() = 0;

        protected:
            ILoaderManager() = default;
    };

    class IResourceManager
    {
        MOUCA_NOCOPY_NOMOVE(IResourceManager);

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

            virtual ~IResourceManager() = default;

        //--------------------------------------------------------------------------
        //								Folder management
        //--------------------------------------------------------------------------
            virtual void addResourceFolder(const Core::Path& folder, const size_t idFolder) = 0;

            virtual const Core::Path& getResourceFolder(const size_t idFolder) const = 0;

        //--------------------------------------------------------------------------
        //							   Resources management
        //--------------------------------------------------------------------------
            virtual void releaseResource(Core::ResourceSPtr&& resource) = 0;

            virtual void releaseResources() = 0;

        //--------------------------------------------------------------------------
        //							  Writable resources
        //--------------------------------------------------------------------------
            virtual Core::FileSPtr createFile(const Core::Path& filename = Core::Path()) = 0;

            virtual XML::ParserSPtr createXML(const Core::Path& filename = Core::Path()) = 0;

            virtual RT::ImageImportSPtr createImage(const Core::Path& filename = Core::Path()) = 0;

            virtual DatabaseSPtr createDatabase() = 0;

        //--------------------------------------------------------------------------
        //						    Readable only resources
        //--------------------------------------------------------------------------
            virtual RT::ShaderFileSPtr openShader( const Core::Path& filename, const RT::ShaderKind kind, const Core::StringOS& sourceShader = Core::StringOS() ) = 0;

            virtual Core::FileSPtr openFile(const Core::Path& filename) = 0;

            virtual XML::ParserSPtr openXML(const Core::Path& filename) = 0;

            virtual RT::ImageImportSPtr openImage(const Core::Path& filename) = 0;

            virtual RT::MeshImportSPtr openMeshImport(const Core::Path& filename, const RT::BufferDescriptor& descriptor = RT::BufferDescriptor(), const RT::MeshImport::Flag flag = RT::MeshImport::ComputeAll) = 0;

            virtual RT::AnimationImporterSPtr openAnimation(const Core::Path& filename) = 0;

        protected:
            IResourceManager() = default;
    };


    ///	\class	CoreSystem
    ///	\brief	Main system: allow to load file/manage thread and all OS problematics
    class ICoreSystem
    {
        MOUCA_NOCOPY_NOMOVE(ICoreSystem);

        public:
            virtual ~ICoreSystem() = default;

            virtual Core::PlugInManager& getPlugInManager() = 0;
            
            virtual Core::ThreadPools& getThreadPools() = 0;
            
            virtual Core::IErrorManager& getExceptionManager() = 0;
            
            virtual IResourceManager& getResourceManager() = 0;

            virtual ILoaderManager& getLoaderManager() = 0;
            
            virtual const Core::Locale& getLocale() const = 0;
            
            virtual void printException(const Core::Exception& exception) const = 0;

        protected:
            ICoreSystem() = default;
    };

    using ICoreSystemUPtr = std::unique_ptr<ICoreSystem>;
}