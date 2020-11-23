/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTBoundingBox.h>
#include <LibRT/include/RTBufferDescriptor.h>

#include <LibVulkan/include/VKBuffer.h>

namespace RT
{
    class Mesh;
}

namespace Vulkan
{
    class CommandBufferOld;
    class Device;

    class Mesh final
    {
        MOUCA_NOCOPY(Mesh);

        public:
            struct Indexed
            {
                //uint32_t _nbVertices = 0;
                uint32_t _startIndex;
                uint32_t _nbIndices;
                int32_t _id;

                Indexed( const uint32_t startIndex=0, const uint32_t nbIndices=0, const int32_t id=0 ):
                _startIndex(startIndex), _nbIndices(nbIndices), _id(id)
                {}
            };

            /// Constructor
            Mesh();
            /// Destructor
            ~Mesh();

            //------------------------------------------------------------------------
            /// \brief  Allocate empty mesh buffer (VAO, IBO)
            /// 
            /// \param[in] device: Vulkan device.
            /// \param[in] vertexSize: size of VAO in byte.
            /// \param[in] indexSize: size of IBO in byte.
            /// \param[in] indexCount: number of items in IBO.
            /// \param[in] GPUOnly: allow to use getEditVertices() and getEditIndices() for transfer memory when false, GPU mode otherwise.
            void initialize(const Device& device, const VkDeviceSize vertexSize, const VkDeviceSize indexSize, const uint32_t indexCount, const bool GPUOnly);

            void release(const Device& device);

            bool isNull() const
            {
                return _vertices.isNull() && _indices.isNull();
            }

            const Buffer& getVertices() const
            {
                return _vertices;
            }

            const Buffer& getIndices() const
            {
                return _indices;
            }

            uint32_t getIndexCount() const
            {
                return _indexCount;
            }

            /// Only for transfer data
            Memory& getEditVertices()
            {
                return _vertices.getMemory();
            }

            /// Only for transfer data
            Memory& getEditIndices()
            {
                return _indices.getMemory();
            }

            const std::vector<Indexed>& getIndexed() const
            {
                return _indexed;
            }

        private:
            struct MeshBufferInfo
            {
                VkBuffer buf        = VK_NULL_HANDLE;
                VkDeviceMemory mem  = VK_NULL_HANDLE;
                size_t size         = 0;
            };

            Buffer                    _vertices;
            Buffer                    _indices;
            uint32_t                  _indexCount;
            std::vector<Indexed>      _indexed;
            
            RT::BoundingBox           _bbox;
    };
}