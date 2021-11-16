/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKAccelerationStructure.h"

#include "LibRT/include/RTMesh.h"

#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKCommandPool.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKMesh.h"

namespace Vulkan
{

AccelerationStructure::AccelerationStructure():
_data(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))
{}

void AccelerationStructure::addGeometry(AccelerationStructureGeometryUPtr&& geometry)
{
    _geometries.emplace_back(std::move(geometry));
}

void AccelerationStructure::setCreateInfo(const VkAccelerationStructureCreateFlagsKHR createFlags, const VkAccelerationStructureTypeKHR type)
{
    // Copy info
    _type = type;
    _createFlags = createFlags;

    MouCa::postCondition(_type != VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR);
}

void AccelerationStructure::initialize(const Device& device)
{
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(_data.isNull());
    MouCa::preCondition(isNull());
    MouCa::preCondition(_type != VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR); // DEV Issue: Missing call setCreateInfo

    // Prepare Build Geometry: need valid buffer with data
    buildGeometry(device);

    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo = _buildGeometry._buildInfo;
    // Buffer and memory
    _data.initialize(device, 0, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                     buildSizeInfo.accelerationStructureSize);

    // Acceleration structure
    const VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,   // VkStructureType                          sType;
        nullptr,                                                    // const void* pNext;
        _createFlags,                                               // VkAccelerationStructureCreateFlagsKHR    createFlags;
        _data.getBuffer(),                                          // VkBuffer                                 buffer;
        0,                                                          // VkDeviceSize                             offset;
        buildSizeInfo.accelerationStructureSize,                    // VkDeviceSize                             size;
        _type,                                                      // VkAccelerationStructureTypeKHR           type;
        0,                                                          // VkDeviceAddress                          deviceAddress;
    };

    if(device.vkCreateAccelerationStructureKHR(device.getInstance(), &accelerationStructureCreate_info, nullptr, &_handle) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "CreateAccelerationStructureError"));
    }

    // AS device address
    const VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, // VkStructureType               sType;
        nullptr,                                                          // const void* pNext;
        _handle,                                                          // VkAccelerationStructureKHR    accelerationStructure;
    };
    _deviceAddress = device.vkGetAccelerationStructureDeviceAddressKHR(device.getInstance(), &accelerationDeviceAddressInfo);

    MouCa::postCondition(!isNull());
}

void AccelerationStructure::release(const Device& device)
{
    MouCa::preCondition(!isNull());

    _data.release(device);

    device.vkDestroyAccelerationStructureKHR(device.getInstance(), _handle, nullptr);
    _handle        = VK_NULL_HANDLE;
    _deviceAddress = 0;
    
    MouCa::postCondition(isNull());
}

AccelerationStructure::BuildGeometry::BuildGeometry() :
_geometryInfo({}), _buildInfo({})
{}

void AccelerationStructure::buildGeometry(const Device& device)
{
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!_geometries.empty());

    // Compute latest info
    uint32_t count = 0;
    _buildGeometry._geometriesVK.reserve(_geometries.size());
    for(const auto& geometry : _geometries)
    {
        geometry->create(device);

        _buildGeometry._geometriesVK.emplace_back(geometry->getGeometry());
        count += geometry->getCount();

        // Not nice : duplication
        for (const auto range : geometry->getRangeInfo())
        {
            _buildGeometry._ranges.emplace_back(range);
        }
    }
    MouCa::assertion(count > 0); // DEV Issue: Check your buffer are properly configured !

    _buildGeometry._geometryInfo =
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,   // sType
        nullptr,                                                            // pNext
        _type,                                                              // type
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,          // flag
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,                     // mode
        nullptr,                                                            // srcAccelerationStructure
        nullptr,                                                            // dstAccelerationStructure
        static_cast<uint32_t>(_buildGeometry._geometriesVK.size()),         // geometryCount
        _buildGeometry._geometriesVK.data(),                                // pGeometries
        nullptr,                                                            // ppGeometries
        { 0 }                                                               // scratchData
    };

    _buildGeometry._buildInfo =
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
    };
    
    device.vkGetAccelerationStructureBuildSizesKHR(device.getInstance(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                   &_buildGeometry._geometryInfo, &count, &_buildGeometry._buildInfo);

    

    MouCa::postCondition(_buildGeometry._buildInfo.accelerationStructureSize > 0);
}

void AccelerationStructure::createAccelerationStructure(const Device& device, std::vector<AccelerationStructureWPtr>& accelerationStructures)
{
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!accelerationStructures.empty());

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    const bool hostCommand = device.getPhysicalDeviceAccelerationStructureFeatures().accelerationStructureHostCommands;

    std::vector<BufferUPtr> scratchBuffers;
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometries;
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos;

    scratchBuffers.reserve(accelerationStructures.size());
    buildGeometries.reserve(accelerationStructures.size());
    accelerationBuildStructureRangeInfos.reserve(accelerationStructures.size());

    for (const auto& weakAS : accelerationStructures)
    {
        auto as = weakAS.lock();
        MouCa::assertion(!as->isNull());
        MouCa::assertion(!as->_buildGeometry._ranges.empty()); //DEV Issue: no data range

        BufferUPtr scratchBuffer = std::make_unique<Buffer>(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR));
        scratchBuffer->initialize(device, 0, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, as->_buildGeometry._buildInfo.buildScratchSize);
        
        // Complete build info
        auto geometryInfo = as->_buildGeometry._geometryInfo;
        geometryInfo.dstAccelerationStructure  = as->getHandle();
        geometryInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress(device);

        buildGeometries.emplace_back(std::move(geometryInfo));
        accelerationBuildStructureRangeInfos.emplace_back(as->_buildGeometry._ranges.data());
        scratchBuffers.emplace_back(std::move(scratchBuffer));
    }

    if (hostCommand)
    {
        // Implementation supports building acceleration structure building on host
        if (device.vkBuildAccelerationStructuresKHR(device.getInstance(), VK_NULL_HANDLE,
            static_cast<uint32_t>(buildGeometries.size()), buildGeometries.data(),
            accelerationBuildStructureRangeInfos.data()) != VK_SUCCESS)
        {
            throw Core::Exception(Core::ErrorData("Vulkan", "BuildAccelerationStructureError"));
        }
    }
    else
    {
        auto commandBuffer = std::make_shared<CommandBuffer>();
        auto pool = std::make_shared<CommandPool>();
        pool->initialize(device, device.getQueueFamilyGraphicId());
        commandBuffer->initialize(device, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);

        commandBuffer->addCommand(std::make_unique<CommandBuildAccelerationStructures>(device, std::move(buildGeometries), std::move(accelerationBuildStructureRangeInfos)));

        commandBuffer->execute();

        std::vector<ICommandBufferWPtr> commandBuffers
        {
            commandBuffer
        };
        device.executeCommandSync(std::move(commandBuffers));

        commandBuffer->release(device);
        pool->release(device);
    }

    for (auto& scratchBuffer : scratchBuffers)
    {
        scratchBuffer->release(device);
    }
}

}