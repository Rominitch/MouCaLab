/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"
#include "LibRT/include/RTMesh.h"

namespace RT
{

Mesh::~Mesh()
{
    BT_POST_CONDITION(isNull());
}

void Mesh::initialize(std::shared_ptr<BufferCPUBase> pVBOBuffer, std::shared_ptr<BufferCPUBase> pIBOBuffer, const SubMeshDescriptors& descriptors, const FaceOrder faceOrder, const Topology topology, const RT::BoundingBox& bbox)
{
    BT_PRE_CONDITION(isNull());
    BT_PRE_CONDITION(faceOrder < FaceOrder::NbFaceOrder);
    BT_PRE_CONDITION(topology  < Topology::NbTopology);

    _VBOBuffer = pVBOBuffer;
    _IBOBuffer = pIBOBuffer;
    _descriptors = descriptors;

    _faceOrder = faceOrder;
    _topology  = topology;
    _bbox      = bbox;

    BT_POST_CONDITION(!isNull());
#ifndef NDEBUG
    // Big data control
    BT_POST_CONDITION(_VBOBuffer->getNbElements() > 0);
    BT_POST_CONDITION(_IBOBuffer->getNbElements() > 0);

    for(const auto& descriptor : _descriptors)
    {
        BT_POST_CONDITION(descriptor._nbVertices <= _VBOBuffer->getNbElements());
        BT_POST_CONDITION(descriptor._startIndex <= _IBOBuffer->getNbElements() * _IBOBuffer->getDescriptor().getComponentDescriptor(0).getNbComponents());
        BT_POST_CONDITION(descriptor._nbIndices  > 0);
        BT_POST_CONDITION((descriptor._startIndex + descriptor._nbIndices) <= _IBOBuffer->getNbElements() * _IBOBuffer->getDescriptor().getComponentDescriptor(0).getNbComponents());
    }
#endif
}

void Mesh::release()
{
    _VBOBuffer.reset();
    _IBOBuffer.reset();
    _descriptors.clear();

    BT_POST_CONDITION(isNull());
}

}