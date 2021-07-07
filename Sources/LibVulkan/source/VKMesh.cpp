/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"
/*
#include "LibRT/include/RTMesh.h"

#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKMesh.h"

namespace Vulkan
{

Mesh::Mesh(const bool GPUOnly):
_indexCount(0),
_vertices(std::make_unique<MemoryBuffer>(GPUOnly ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)),
_indices(std::make_unique<MemoryBuffer>(GPUOnly ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
{
    MOUCA_PRE_CONDITION(isNull());
}

Mesh::~Mesh()
{
    MOUCA_PRE_CONDITION(isNull());
}

void Mesh::initialize(const Device& device, const uint32_t nbVertices, const VkDeviceSize vertexStride, const VkDeviceSize indexSize, const uint32_t indexCount)
{
    MOUCA_PRE_CONDITION(isNull());
    _nbVertices   = nbVertices;
    _vertexStride = vertexStride;

    const VkDeviceSize vertexSize = nbVertices * vertexStride;
    MOUCA_PRE_CONDITION(vertexSize > 0 && indexSize > 0);
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(indexCount > 0);

    // Allocate GPU buffers
    _vertices.initialize(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         vertexSize);
    _indices.initialize(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        indexSize);
    _indexCount = indexCount;

    MOUCA_POST_CONDITION(!isNull());
}

void Mesh::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());

    _vertices.release(device);

    _indices.release(device);

    MOUCA_ASSERT(isNull());
}

}
*/