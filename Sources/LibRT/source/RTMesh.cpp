/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"
#include "LibRT/include/RTMesh.h"

namespace RT
{

Mesh::~Mesh()
{
    MOUCA_POST_CONDITION(isNull());
}

void Mesh::initialize(std::shared_ptr<BufferCPUBase> pVBOBuffer, std::shared_ptr<BufferCPUBase> pIBOBuffer, const SubMeshDescriptors& descriptors, const FaceOrder faceOrder, const Topology topology, const RT::BoundingBox& bbox)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(faceOrder < FaceOrder::NbFaceOrder);
    MOUCA_PRE_CONDITION(topology  < Topology::NbTopology);

    _VBOBuffer = pVBOBuffer;
    _IBOBuffer = pIBOBuffer;
    _descriptors = descriptors;

    _faceOrder = faceOrder;
    _topology  = topology;
    _bbox      = bbox;

    MOUCA_POST_CONDITION(!isNull());
#ifndef NDEBUG
    // Big data control
    MOUCA_POST_CONDITION(_VBOBuffer->getNbElements() > 0);
    MOUCA_POST_CONDITION(_IBOBuffer->getNbElements() > 0);

    for(const auto& descriptor : _descriptors)
    {
        MOUCA_POST_CONDITION(descriptor._nbVertices <= _VBOBuffer->getNbElements());
        MOUCA_POST_CONDITION(descriptor._startIndex <= _IBOBuffer->getNbElements() * _IBOBuffer->getDescriptor().getComponentDescriptor(0).getNbComponents());
        MOUCA_POST_CONDITION(descriptor._nbIndices  > 0);
        MOUCA_POST_CONDITION((descriptor._startIndex + descriptor._nbIndices) <= _IBOBuffer->getNbElements() * _IBOBuffer->getDescriptor().getComponentDescriptor(0).getNbComponents());
    }
#endif
}

void Mesh::release()
{
    _VBOBuffer.reset();
    _IBOBuffer.reset();
    _descriptors.clear();

    MOUCA_POST_CONDITION(isNull());
}

}