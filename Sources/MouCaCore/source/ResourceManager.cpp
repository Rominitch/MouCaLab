#include "Dependencies.h"

#include "MouCaCore/include/ResourceManager.h"

#include "MouCaCore/include/DatabaseManagerSqlite.h"

#include <LibRT/include/RTAnimationBones.h>
#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTShaderFile.h>

#include <LibMedia/include/ImageFI.h>

#include <LibXML/include/XMLParser.h>

namespace MouCaCore
{

void ResourceManager::addResourceFolder(const Core::Path& folder, const size_t idFolder)
{
    MOUCA_PRE_CONDITION(std::filesystem::exists(folder)); // DEV Issue: Dev must create manually folder.
    MOUCA_PRE_CONDITION(_mapFolders.find(idFolder) == _mapFolders.cend());

    _mapFolders[idFolder] = folder;
}

void ResourceManager::registerResource(Core::ResourceSPtr resource)
{
    MOUCA_PRE_CONDITION(_resources.find(resource) == _resources.cend());
    _resources.insert(resource);
}

void ResourceManager::releaseResources()
{
    for(auto& resource : _resources)
    {
        MOUCA_PRE_CONDITION( resource.use_count() == 1 );
        resource->release();
    }

    _resources.clear();
}

void ResourceManager::releaseResource(Core::ResourceSPtr&& resource)
{
    const auto instance = resource.use_count();
    MOUCA_ASSERT(instance >= 2); // DEV Issue: Need this instance + manager !

    // Keep pointer
    Core::Resource* data = resource.get(); // If error here: Do you forget include ?
    resource.reset(); // Release current shared

    // Latest resource is own by manager
    if (instance == 2)
    {
        // Release data inside resource
        if (!data->isNull())
        {
            data->release();
        }
        // Final unregister and delete
        unregisterResource(data);
    }
}

Core::FileSPtr ResourceManager::createFile(const Core::Path& filename)
{
    auto file = std::make_shared<Core::File>();
    _resources.insert(file);
    if(!filename.empty())
    {
        file->setFileInfo(filename);
    }
    return file;
}

XML::ParserSPtr ResourceManager::createXML(const Core::Path& filename)
{
    auto file = std::make_shared<XML::XercesParser>();
    _resources.insert(file);
    if(!filename.empty())
    {
        file->setFileInfo(filename);
    }
    return file;
}

RT::ImageImportSPtr ResourceManager::createImage(const Core::Path& filename)
{
    auto imageImport = std::make_shared<RT::ImageImport>();
    _resources.insert(imageImport);
    if(!filename.empty())
    {
        imageImport->setFileInfo(filename);
    }

    imageImport->setImage(std::make_shared<Media::ImageFI>());

    return imageImport;
}

DatabaseSPtr ResourceManager::createDatabase()
{
	auto db = std::make_shared<DatabaseManagerSqlite>();
    _resources.insert(db);
    return db;
}

template<typename BuildClass>
std::shared_ptr<BuildClass> ResourceManager::genericOpen( const Core::Path& filename )
{
    MOUCA_PRE_CONDITION( !filename.empty() );
    MOUCA_PRE_CONDITION( std::filesystem::exists( filename ) );

    std::shared_ptr<BuildClass> file;
    Core::ResourceSPtr res = searchResources<BuildClass>( filename );
    if( res != nullptr )
    {
        file = std::static_pointer_cast<BuildClass>(res);
        MOUCA_ASSERT(file != nullptr);
    }
    else
    {
        file = std::make_shared<BuildClass>();
        _resources.insert( file );
        file->setFileInfo( filename );
    }
    return file;
}

Core::FileSPtr ResourceManager::openFile(const Core::Path& filename)
{
    return genericOpen<Core::File>( filename );
}

XML::ParserSPtr ResourceManager::openXML(const Core::Path& filename)
{
    return genericOpen<XML::XercesParser>( filename );
}

RT::ImageImportSPtr ResourceManager::openImage(const Core::Path& filename)
{
    return genericOpen<RT::ImageImport>( filename );
}

RT::MeshImportSPtr ResourceManager::openMeshImport(const Core::Path& filename, const RT::BufferDescriptor& descriptor, const RT::MeshImport::Flag flag)
{
    MOUCA_PRE_CONDITION(!filename.empty());
    MOUCA_PRE_CONDITION(std::filesystem::exists(filename));

    RT::MeshImportSPtr file;
    Core::ResourceSPtr res = searchResources<RT::MeshImport>(filename);
    if (res != nullptr)
    {
        file = std::static_pointer_cast<RT::MeshImport>(res);
        MOUCA_ASSERT(file != nullptr);
    }
    else
    {
        file = std::make_shared<RT::MeshImport>();
        _resources.insert( file );
        file->setFileInfo( filename, descriptor, flag );
    }
    return file;
}

RT::ShaderFileSPtr ResourceManager::openShader( const Core::Path& filePath, const RT::ShaderKind kind, const Core::StringOS& sourceShader )
{
    MOUCA_PRE_CONDITION( !filePath.empty() );

    // Build default Spir-V filepath
    const Core::Path realFilePath = std::filesystem::absolute(
                                    std::filesystem::exists( filePath ) 
                                    ? filePath
                                    : std::filesystem::path(getResourceFolder(ResourceFolder::Shaders) / std::filesystem::path(filePath)));
    
    MOUCA_PRE_CONDITION( std::filesystem::exists( realFilePath ) );

    // Build source default path
    Core::StringOS realSourcePath;
    if (!sourceShader.empty())
    {
        realSourcePath = std::filesystem::absolute(
                       std::filesystem::exists(sourceShader)
                       ? sourceShader
                       : std::filesystem::path(getResourceFolder(ResourceFolder::ShadersSource) / std::filesystem::path(sourceShader)));

        MOUCA_PRE_CONDITION(std::filesystem::exists(realSourcePath));
    }

    RT::ShaderFileSPtr file;
    Core::ResourceSPtr res = searchResources<RT::ShaderFile>( realFilePath );
    if( res != nullptr )
    {
        file = std::static_pointer_cast<RT::ShaderFile>(res);
        MOUCA_ASSERT(file != nullptr);
    }
    else
    {
        file = std::make_shared<RT::ShaderFile>( realFilePath, kind, realSourcePath );
        _resources.insert( file );
    }
    return file;
}

RT::AnimationImporterSPtr ResourceManager::openAnimation(const Core::Path& filename)
{
    return genericOpen<RT::AnimationImporter>(filename);
}

template<typename ResourceType>
Core::ResourceSPtr ResourceManager::searchResources(const Core::Path& filename) const
{
    MOUCA_PRE_CONDITION(!filename.empty());
    MOUCA_PRE_CONDITION(std::filesystem::exists(filename));

    // Find resource by name
    const auto& itResource = std::find_if(_resources.cbegin(), _resources.cend(),
                                          [&](const auto& resource)
                                          {
                                              return resource->getFilename() == filename
                                                  && std::dynamic_pointer_cast<ResourceType>( resource );
                                          });
    return (itResource != _resources.cend()) ? *itResource : nullptr;
}

}