/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTBBoxes.h"

namespace RT
{

void BBoxes::initialize()
{
    MOUCA_PRE_CONDITION( isNull() );

    // Build bbox mesh : centered bbox of 1 size
    _nodes =
    {
        RT::Point3(-0.5f, -0.5f , -0.5f),
        RT::Point3(-0.5f, -0.5f ,  0.5f),
        RT::Point3(-0.5f,  0.5f ,  0.5f),
        RT::Point3(-0.5f,  0.5f , -0.5f),        

        RT::Point3( 0.5f, -0.5f , -0.5f),
        RT::Point3( 0.5f, -0.5f ,  0.5f),
        RT::Point3( 0.5f,  0.5f ,  0.5f),
        RT::Point3( 0.5f,  0.5f , -0.5f)        
    };
    _edges =
    {
        Edge( 0, 1 ),
        Edge( 1, 2 ),
        Edge( 2, 3 ),
        Edge( 3, 0 ),

        Edge( 4, 5 ),
        Edge( 5, 6 ),
        Edge( 6, 7 ),
        Edge( 7, 4 ),

        Edge( 0, 4 ),
        Edge( 1, 5 ),
        Edge( 2, 6 ),
        Edge( 3, 7 )
    };

    const std::vector<RT::ComponentDescriptor> descriptors =
    {
        { 3, RT::Type::Float, RT::ComponentUsage::Vertex  }
    };
    RT::BufferDescriptor bufferDescriptor;
    bufferDescriptor.initialize( descriptors );

    const std::vector<RT::ComponentDescriptor> descriptorsIBO =
    {
        { 2, RT::Type::UnsignedInt, RT::ComponentUsage::Index }
    };
    RT::BufferDescriptor bufferDescriptorIBO;
    bufferDescriptorIBO.initialize( descriptorsIBO );

    RT::BufferLinkedCPU* linkedVertices = new RT::BufferLinkedCPU();
    linkedVertices->create( bufferDescriptor, _nodes.size(), _nodes.data() );
    std::shared_ptr<RT::BufferCPUBase> vbo( linkedVertices );

    RT::BufferLinkedCPU* linkedIBO = new RT::BufferLinkedCPU();
    linkedIBO->create( bufferDescriptorIBO, _edges.size(), _edges.data() );
    std::shared_ptr<RT::BufferCPUBase> ibo( linkedIBO );

    const RT::Mesh::SubMeshDescriptors subDescriptors =
    {
        { _nodes.size(), 0, _edges.size() * 2, 0 }
    };
    _bbox.initialize( vbo, ibo, subDescriptors, RT::FaceOrder::CounterClockWise, RT::Topology::Lines, RT::BoundingBox(_nodes[0], _nodes[7]) );

    MOUCA_POST_CONDITION( !isNull() );
}

void BBoxes::release()
{
    MOUCA_PRE_CONDITION( !isNull() );

    _bbox.release();

    BasicMassiveInstance::release();

    MOUCA_POST_CONDITION( isNull() );
}

bool BBoxes::isNull() const
{
    return _bbox.isNull() && BasicMassiveInstance::isNull();
}

void BBoxes::update( const std::vector<Indirect>& indirects, const std::vector<Instance>& instances )
{
    MOUCA_PRE_CONDITION( !isNull() );               // DEV Issue: Mesh are set !
    MOUCA_PRE_CONDITION( 1 == indirects.size() );   // DEV Issue: Need same size to parse easily ! 

    // Copy data
    _indirects = indirects;
    _instances = instances;

    // Read mesh data
    for( auto& indirect : _indirects )
    {
        indirect._indexCount = static_cast<uint32_t>(_bbox.getIBOBuffer().lock()->getByteSize());
    }

#ifdef _DEBUG
    // Control data: check nb of instances is correctly sync with indirect
    const uint32_t nbObjects = std::accumulate(_indirects.cbegin(), _indirects.cend(), 0,
                                                 [](uint32_t count, const Indirect& indirect) { return count + indirect._count; });
    MOUCA_POST_CONDITION( _instances.size() == nbObjects );
#endif
}

}