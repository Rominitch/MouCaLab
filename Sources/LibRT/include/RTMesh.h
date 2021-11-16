/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreResource.h>

#include <LibRT/include/RTBoundingBox.h>
#include <LibRT/include/RTBufferCPU.h>

namespace RT
{
    class Mesh final
    {
        public:
            struct SubMeshDescriptor
            {
                size_t _nbVertices  = 0;

                size_t _startIndex  = 0;    ///< Indices position.           Example _startIndex = 3 * triangles.
                size_t _nbIndices   = 0;    ///< Number of indices for mesh. Example _nbIndices  = 3 * triangles.
                uint8_t _material = 0;
            };
            using SubMeshDescriptors = std::vector<SubMeshDescriptor>;

            Mesh() = default;

            ~Mesh();

            bool isNull() const
            {
                return _VBOBuffer == nullptr 
                    && _IBOBuffer == nullptr
                    && _descriptors.empty();
            }

            void initialize(std::shared_ptr<BufferCPUBase> pVBOBuffer, std::shared_ptr<BufferCPUBase> pIBOBuffer, const SubMeshDescriptors& descriptors, const FaceOrder faceOrder = FaceOrder::CounterClockWise, const Topology topology = Topology::Triangles, const RT::BoundingBox& bbox=RT::BoundingBox());

            void release();

            const std::weak_ptr<BufferCPUBase> getVBOBuffer() const
            {
                return _VBOBuffer;
            }

            const std::weak_ptr<BufferCPUBase> getIBOBuffer() const
            {
                return _IBOBuffer;
            }

            size_t getNbVertices() const
            {
                return _VBOBuffer->getNbElements();
            }

            size_t getNbPolygones() const
            {
                return _IBOBuffer->getNbElements();
            }

            const SubMeshDescriptors& getDescriptors() const
            {
                return _descriptors;
            }

            FaceOrder getFaceOrder() const
            {
                return _faceOrder;
            }

            void setFaceOrder(const FaceOrder order)
            {
                MouCa::preCondition(order < FaceOrder::NbFaceOrder);
                _faceOrder = order;
            }

            Topology getTopology() const
            {
                return _topology;
            }

            const BoundingBox& getBoundingBox() const
            {
                return _bbox;
            }

            //--------------------------------------------------------------------------
            //								Global format
            //--------------------------------------------------------------------------
            struct SGeometry
            {
                Point3 vertex;
                Point2 texCoord;
                Point3 normal;
                Point3 tangent;
            };
            struct SIndex
            {
                Array3ui iIndex;
            };

            static BufferDescriptor getStandardVBOFormat()
            {
                const std::vector<ComponentDescriptor> descriptors =
                {
                    {3, Type::Float, ComponentUsage::Vertex},
                    {2, Type::Float, ComponentUsage::TexCoord0},
                    {3, Type::Float, ComponentUsage::Normal},
                    {3, Type::Float, ComponentUsage::Tangent}
                };

                MouCa::assertion(sizeof(Mesh::SGeometry) == descriptors[0].getSizeInByte() + descriptors[1].getSizeInByte() + descriptors[2].getSizeInByte() + descriptors[3].getSizeInByte());

                //Buffer Descriptor
                //static const size_t					                        szNbDescriptors = 2;
                //const std::array<size_t, szNbDescriptors>                   szDescriptor    = {3, 2};
                //const std::array<RT::enumRTType, szNbDescriptors>			eType           = {RT::TypeFloat, RT::TypeFloat};
                //const std::array<RT::enumBufferComponent, szNbDescriptors>	eComponantType  = {RT::BufferVertices, RT::BufferTexCoord0};
                //const std::array<bool, szNbDescriptors>						pNormalized     = {false, false};

                BufferDescriptor bufferDescriptor;
                bufferDescriptor.initialize(descriptors);
                return bufferDescriptor;
            }

            static BufferDescriptor getStandardIBOFormat()
            {
                const std::vector<ComponentDescriptor> descriptors =
                {
                    {3, Type::UnsignedInt, ComponentUsage::Index}
                };

                MouCa::assertCompare(sizeof(Mesh::SIndex), descriptors[0].getSizeInByte());

                BufferDescriptor BufferDescriptorIBO;
                BufferDescriptorIBO.initialize(descriptors);
                return BufferDescriptorIBO;
            }

        private:
            std::shared_ptr<BufferCPUBase>	_VBOBuffer;
            std::shared_ptr<BufferCPUBase>	_IBOBuffer;

            SubMeshDescriptors              _descriptors;

            BoundingBox                     _bbox;                                      ///< Local axis align bounding box.
            FaceOrder                       _faceOrder  = FaceOrder::CounterClockWise;  ///< Representing face order.
            Topology                        _topology   = Topology::Triangles;          ///< Representing of buffer data.

            MOUCA_NOCOPY(Mesh);
    };
    using MeshSPtr = std::shared_ptr<Mesh>;
    using MeshWPtr = std::weak_ptr<Mesh>;

    //----------------------------------------------------------------------------
    /// \brief Resource which can store all data to load mesh buffers.
    /// 
    /// \see Core::Resource
    class MeshImport : public Core::Resource
    {
        MOUCA_NOCOPY_NOMOVE(MeshImport);

        public:
            enum Flag
            {
                DefaultImport  = 0x0000,
                ComputeNormal  = 0x0001,
                ComputeTangent = 0x0002,
                InvertY        = 0x0004,

                ComputeAll       = ComputeNormal | ComputeTangent,
                ComputeAllInvert = ComputeAll | InvertY,

                SpecialExport  = 0x1000
            };

            MeshImport() :
            _flag( DefaultImport ), _mesh(std::make_shared<Mesh>())
            {
                MouCa::preCondition( isNull() );
            }

            ~MeshImport() override
            {
                MouCa::preCondition( !isLoaded() );
            }

            void setFileInfo(const Core::Path& filename, const BufferDescriptor& descriptor, const Flag flag)
            {
                MouCa::preCondition( isNull() );
                
                Core::Resource::setFileInfo(filename);

                _descriptor = descriptor;
                _flag       = flag;

                MouCa::postCondition( !isNull() );
            }

            void release() override
            {
                MouCa::preCondition( !isNull() );

                _filename.clear();
                _mesh->release();
                _mesh.reset();

                MouCa::preCondition( isNull() );
            }

            bool isNull() const override
            {
                return _filename.empty() && !isLoaded();
            }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is loaded.
            /// 
            /// \returns True if loaded, otherwise false.
            bool isLoaded() const override
            {
                return _mesh && !_mesh->isNull();
            }

            MeshWPtr getWeakMesh() const
            {
                return _mesh;
            }

            const Mesh& getMesh() const
            {
                return *_mesh;
            }

            Mesh& getEditMesh()
            {
                return *_mesh;
            }

            const BufferDescriptor& getDescriptor() const
            {
                return _descriptor;
            }

            const Flag getFlag() const
            {
                return _flag;
            }

        protected:
            BufferDescriptor _descriptor;
            Flag             _flag;

            MeshSPtr         _mesh;
    };
}