/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibRT/include/RTMassiveInstance.h>

namespace RT
{

void BasicMassiveInstance::release()
{
    _instances.clear();
    _indirects.clear();
}

void MassiveInstance::addMesh( const MeshImportWPtr& mesh )
{
    _meshes.emplace_back( mesh );
}

void MassiveInstance::addImage( const RT::ImageWPtr& image )
{
    _images.emplace_back( image );
}

void MassiveInstance::release()
{
    _meshes.clear();

    BasicMassiveInstance::release();

    MOUCA_POST_CONDITION( isNull() );
}

void MassiveInstance::update( const std::vector<Indirect>& indirects, const std::vector<Instance>& instances )
{
    MOUCA_PRE_CONDITION( !isNull() );                            // DEV Issue: Mesh are set !
    MOUCA_PRE_CONDITION( _meshes.size() == indirects.size() );   // DEV Issue: Need same size to parse easily ! 

    // Copy data
    _indirects = indirects;
    _instances = instances;

    // Read mesh data
    auto itMesh = _meshes.cbegin();
    for( auto& indirect : _indirects )
    {
        indirect._indexCount = static_cast<uint32_t>(itMesh->lock()->getMesh().getIBOBuffer().lock()->getByteSize());
        ++itMesh;
    }

#ifdef _DEBUG
    // Control data: check nb of instances is correctly sync with indirect
    const uint32_t nbObjects = std::accumulate(_indirects.cbegin(), _indirects.cend(), 0,
                                                 [](uint32_t count, const Indirect& indirect) { return count + indirect._count; });
    MOUCA_POST_CONDITION( _instances.size() == nbObjects );
#endif
}

}