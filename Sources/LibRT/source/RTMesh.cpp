/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"
#include "LibRT/include/RTMesh.h"

namespace RT
{

Mesh::~Mesh()
{
    MouCa::postCondition(isNull());
}

void Mesh::initialize(std::shared_ptr<BufferCPUBase> pVBOBuffer, std::shared_ptr<BufferCPUBase> pIBOBuffer, const SubMeshDescriptors& descriptors, const FaceOrder faceOrder, const Topology topology, const RT::BoundingBox& bbox)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(faceOrder < FaceOrder::NbFaceOrder);
    MouCa::preCondition(topology  < Topology::NbTopology);

    _VBOBuffer = pVBOBuffer;
    _IBOBuffer = pIBOBuffer;
    _descriptors = descriptors;

    _faceOrder = faceOrder;
    _topology  = topology;
    _bbox      = bbox;

    MouCa::postCondition(!isNull());
#ifndef NDEBUG
    // Big data control
    MouCa::postCondition(_VBOBuffer->getNbElements() > 0);
    MouCa::postCondition(_IBOBuffer->getNbElements() > 0);

    for(const auto& descriptor : _descriptors)
    {
        MouCa::postCondition(descriptor._nbVertices <= _VBOBuffer->getNbElements());
        MouCa::postCondition(descriptor._startIndex <= _IBOBuffer->getNbElements() * _IBOBuffer->getDescriptor().getComponentDescriptor(0).getNbComponents());
        MouCa::postCondition(descriptor._nbIndices  > 0);
        MouCa::postCondition((descriptor._startIndex + descriptor._nbIndices) <= _IBOBuffer->getNbElements() * _IBOBuffer->getDescriptor().getComponentDescriptor(0).getNbComponents());
    }
#endif
}

void Mesh::release()
{
    _VBOBuffer.reset();
    _IBOBuffer.reset();
    _descriptors.clear();

    MouCa::postCondition(isNull());
}

}