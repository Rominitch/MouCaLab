/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

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

    MouCa::postCondition( isNull() );
}

void MassiveInstance::update( const std::vector<Indirect>& indirects, const std::vector<Instance>& instances )
{
    MouCa::preCondition( !isNull() );                            // DEV Issue: Mesh are set !
    MouCa::preCondition( _meshes.size() == indirects.size() );   // DEV Issue: Need same size to parse easily ! 

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
    MouCa::postCondition( _instances.size() == nbObjects );
#endif
}

}