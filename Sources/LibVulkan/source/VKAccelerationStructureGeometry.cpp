#include "Dependencies.h"

#include "LibVulkan/include/VKAccelerationStructureGeometry.h"

#include "LibRT/include/RTMesh.h"

#include "LibVulkan/include/VKAccelerationStructure.h"
#include "LibVulkan/include/VKContextDevice.h"

namespace Vulkan
{

AccelerationStructureGeometry::AccelerationStructureGeometry(const VkGeometryTypeKHR type):
_geometry(
{
    VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
    nullptr,
    type,
    {},
    0
})
{}

AccelerationStructureGeometryInstance::AccelerationStructureGeometryInstance():
AccelerationStructureGeometry(VK_GEOMETRY_TYPE_INSTANCES_KHR),
_data(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))
{}

void AccelerationStructureGeometry::initialize(const VkGeometryFlagsKHR geometryFlags)
{
    _geometry.flags = geometryFlags;
}

void AccelerationStructureGeometryInstance::create(const Device& device)
{
    MouCa::preCondition(!device.isNull());

    // Copy data to buffer
    std::vector<VkAccelerationStructureInstanceKHR> data;
    data.reserve(_instances.size());
    for (auto& instance : _instances)
    {
        data.emplace_back(instance.compute());
    }

    _data.initialize(device, 0, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        sizeof(VkAccelerationStructureInstanceKHR) * data.size(), data.data());

    _geometry.geometry.instances =
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,   // VkStructureType                  sType;
        nullptr,                                                                // const void* pNext;
        VK_FALSE,                                                               // VkBool32                         arrayOfPointers;
        _data.getDeviceAddress(device),                                         // VkDeviceOrHostAddressConstKHR    data;
    };

    _count = 1;

    VkAccelerationStructureBuildRangeInfoKHR info
    {
        1,              //uint32_t    primitiveCount
        0,              //uint32_t    primitiveOffset
        0,              //uint32_t    firstVertex
        0,              //uint32_t    transformOffset
    };
    _rangeInfos.emplace_back(std::move(info));
}

void AccelerationStructureGeometryInstance::Instance::initialize(AccelerationStructureWPtr reference, const VkGeometryInstanceFlagsKHR flag, VkTransformMatrixKHR&& transform4x3,
                                                                 const uint32_t instanceCustomIndex, const uint32_t instanceShaderBinding, const uint32_t mask)
{
    MouCa::preCondition(!reference.expired());

    _reference = reference;
    _instance =
    {
        transform4x3,                           //VkTransformMatrixKHR          transform;
        instanceCustomIndex,                    //uint32_t                      instanceCustomIndex : 24;
        mask ,                                  //uint32_t                      mask : 8;
        instanceShaderBinding,                  //uint32_t                      instanceShaderBindingTableRecordOffset : 24;
        flag,                                   //VkGeometryInstanceFlagsKHR    flags : 8;
        _reference.lock()->getDeviceAdress(),   //uint64_t                      accelerationStructureReference;
    };
}

const VkAccelerationStructureInstanceKHR& AccelerationStructureGeometryInstance::Instance::compute()
{
    // Update reference
    _instance.accelerationStructureReference = _reference.lock()->getDeviceAdress();

    return _instance;
}

AccelerationStructureGeometryTriangles::AccelerationStructureGeometryTriangles():
AccelerationStructureGeometry(VK_GEOMETRY_TYPE_TRIANGLES_KHR)
{}

void AccelerationStructureGeometryTriangles::initialize(const RT::MeshWPtr mesh, const BufferWPtr vbo, const BufferWPtr ibo,
                                                        const VkGeometryFlagsKHR geometryFlags)
{
    AccelerationStructureGeometry::initialize(geometryFlags);

    _mesh = mesh;
    _vbo  = vbo;
    _ibo  = ibo;
}

void AccelerationStructureGeometryTriangles::create(const Device& device)
{
    MouCa::preCondition(!device.isNull());

    const auto mesh = _mesh.lock();
    const uint64_t vertexStride = mesh->getVBOBuffer().lock()->getDescriptor().getByteSize();

    _geometry.geometry.triangles = 
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,  // type
        nullptr,                                                               // next
        VK_FORMAT_R32G32B32_SFLOAT,                                            // vertexFormat
        { _vbo.lock()->getDeviceAddress(device) },                             // vertexData
        static_cast<uint32_t>(vertexStride),                                   // vertexStride
        static_cast<uint32_t>(mesh->getNbVertices()),                         // maxVertex
        VK_INDEX_TYPE_UINT32,                                                  // indexType
        { _ibo.lock()->getDeviceAddress(device) },                             // indexData
        { 0 },                                                                 // transformData
        
    };

    _count = static_cast<uint32_t>(mesh->getNbPolygones());

    VkAccelerationStructureBuildRangeInfoKHR info
    {
        _count,         //uint32_t    primitiveCount
        0,              //uint32_t    primitiveOffset
        0,              //uint32_t    firstVertex
        0,              //uint32_t    transformOffset
    };
    _rangeInfos.emplace_back(std::move(info));
}

}