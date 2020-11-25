/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibRT/include/RTBufferCPU.h>
#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTMesh.h>

namespace RT
{

void Geometry::addImage(const RT::ImageWPtr& image)
{
    _images.emplace_back(image);
}

void GeometryExternal::initialize(const MeshImportWPtr& mesh, const BoundingBox& bbox)
{
    MOUCA_PRE_CONDITION(!mesh.lock()->isNull());

    _mesh        = mesh;
    _boundingBox = bbox;
}

const Mesh& GeometryExternal::getMesh() const
{
    return _mesh.lock()->getMesh();
}

void GeometryInternal::initialize(const MeshSPtr& mesh, const BoundingBox& bbox)
{
    MOUCA_PRE_CONDITION(!mesh->isNull());
    MOUCA_PRE_CONDITION(bbox.isValid());

    _mesh = mesh;
    _boundingBox = bbox;

    MOUCA_POST_CONDITION(!_mesh->isNull());
    MOUCA_POST_CONDITION(_boundingBox.isValid());
}

void GeometryInternal::release()
{
    MOUCA_PRE_CONDITION(_mesh.use_count() <= 1); // DEV issue: latest instance of ownership ?
    
    if(_mesh != nullptr)
    {
        _mesh->release();
        _mesh.reset();
    }
    _images.clear();

    MOUCA_POST_CONDITION(_mesh == nullptr);
}
/*
void Geometry::Export(const Core::Path& strFileName) const
{
    std::ofstream file;
    file.open(strFileName, std::ofstream::out);

    std::shared_ptr<BufferCPUBase> IBOBuffer = _pMesh->getIBOBuffer().lock();
    unsigned int* pI = (unsigned int*)IBOBuffer->lock();
    file << "IBO"<< std::endl;
    for(size_t szIBO=0; szIBO < IBOBuffer->getNbElements(); ++szIBO)
    {
        file << (pI[szIBO*3]) << " " << (pI[szIBO*3+1]) << " " << (pI[szIBO*3+2]) << std::endl;
    }	
    IBOBuffer->unlock();

    std::shared_ptr<BufferCPUBase> VAOBuffer = _pMesh->getVBOBuffer().lock();
    Mesh::SGeometry* pV = reinterpret_cast<Mesh::SGeometry*>(VAOBuffer->lock());
    file << "VAO"<< std::endl;
    for(size_t szVAO=0; szVAO < VAOBuffer->getNbElements(); ++szVAO)
    {
        file	<< pV[szVAO].vertex[0]   << " " << pV[szVAO].vertex[1]  << " " << pV[szVAO].vertex[2] << "\t-\t"
                << pV[szVAO].normal[0]   << " " << pV[szVAO].normal[1]  << pV[szVAO].normal[2] << "\t-\t"
                << pV[szVAO].texCoord[0] << " " << pV[szVAO].texCoord[1]<< std::endl;
    }
    VAOBuffer->unlock();
}
*/

}