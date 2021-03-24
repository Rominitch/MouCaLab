/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKAccelerationStructure.h"

#include "LibRT/include/RTMesh.h"

#include "LibVulkan/include/VKAccelerationStructureGeometry.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKCommandPool.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKMesh.h"

namespace Vulkan
{

// TODO("TMP declaration - Clean")

AccelerationStructure::AccelerationStructure():
_data(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))
{}

void AccelerationStructure::initialize(const ContextDevice& context, const VkAccelerationStructureCreateFlagsKHR createFlags, const VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
    MOUCA_PRE_CONDITION(!context.isNull());
    MOUCA_PRE_CONDITION(_data.isNull());
    MOUCA_PRE_CONDITION(_buildSizeInfo.accelerationStructureSize > 0);  //Dev Issue: Nothing to allocate ?
    MOUCA_PRE_CONDITION(isNull());

    const Device& device = context.getDevice();
    
    // Copy info
    _buildSizeInfo = buildSizeInfo;
    _type          = type;

    // Buffer and memory
    _data.initialize(device, 0, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                     _buildSizeInfo.accelerationStructureSize);

    // Acceleration structure
    const VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,   // VkStructureType                          sType;
        nullptr,                                                    // const void* pNext;
        createFlags,                                                // VkAccelerationStructureCreateFlagsKHR    createFlags;
        _data.getBuffer(),                                          // VkBuffer                                 buffer;
        0,                                                          // VkDeviceSize                             offset;
        _buildSizeInfo.accelerationStructureSize,                   // VkDeviceSize                             size;
        _type,                                                      // VkAccelerationStructureTypeKHR           type;
        0,                                                          // VkDeviceAddress                          deviceAddress;
    };

    if(device.vkCreateAccelerationStructureKHR(device.getInstance(), &accelerationStructureCreate_info, nullptr, &_handle) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"CreateAccelerationStructureError");
    }

    // AS device address
    const VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, // VkStructureType               sType;
        nullptr,                                                          // const void* pNext;
        _handle,                                                          // VkAccelerationStructureKHR    accelerationStructure;
    };
    _deviceAddress = device.vkGetAccelerationStructureDeviceAddressKHR(device.getInstance(), &accelerationDeviceAddressInfo);

    MOUCA_POST_CONDITION(!isNull());
}

void AccelerationStructure::build(const ContextDevice& context, std::vector<AccelerationBuildGeometry>& builds)
{
    MOUCA_PRE_CONDITION(!context.isNull());
    MOUCA_PRE_CONDITION(!isNull());

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    const Device& device = context.getDevice();
    const bool hostCommand = device.getPhysicalDeviceAccelerationStructureFeatures().accelerationStructureHostCommands;

    std::vector<BufferUPtr> scratchBuffers;
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometries;
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos;
    
    scratchBuffers.reserve(builds.size());
    buildGeometries.reserve(builds.size());
    accelerationBuildStructureRangeInfos.reserve(builds.size());
    
    for(const auto& info : builds)
    {
        BufferUPtr scratchBuffer = std::make_unique<Buffer>(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR));
        scratchBuffer->initialize(device, 0, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, info.getBuildInfo().buildScratchSize);

        auto buildInfo = info.getGeometryInfo();
        // Complete build info
        buildInfo.dstAccelerationStructure = _handle;
        buildInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress(device);

        buildGeometries.emplace_back(std::move(buildInfo));
        accelerationBuildStructureRangeInfos.emplace_back(info.getRanges().data());
        scratchBuffers.emplace_back(std::move(scratchBuffer));
    }

    if (hostCommand)
    {
        // Implementation supports building acceleration structure building on host
        if(device.vkBuildAccelerationStructuresKHR( device.getInstance(), VK_NULL_HANDLE, 
            static_cast<uint32_t>(buildGeometries.size()), buildGeometries.data(),
            accelerationBuildStructureRangeInfos.data()) != VK_SUCCESS)
        {
            MOUCA_THROW_ERROR(u8"Vulkan", u8"BuildAccelerationStructureError");
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

    for(auto& scratchBuffer : scratchBuffers)
    {
        scratchBuffer->release(device);
    }

    MOUCA_POST_CONDITION(!isNull());
}

void AccelerationStructure::release(const ContextDevice& context)
{
    MOUCA_PRE_CONDITION(!isNull());

    const Device& device = context.getDevice();
    
    _data.release(device);

    device.vkDestroyAccelerationStructureKHR(device.getInstance(), _handle, nullptr);
    _handle        = VK_NULL_HANDLE;
    _deviceAddress = 0;
    
    MOUCA_POST_CONDITION(isNull());
}

void AccelerationBuildGeometry::initialize(const ContextDevice& context)
{
    MOUCA_PRE_CONDITION(!context.isNull());
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!_geometries.empty());

    // Compute latest info
    uint32_t count = 0;
    std::vector<VkAccelerationStructureGeometryKHR> geometriesVK;
    geometriesVK.reserve(_geometries.size());
    for(const auto& geometry : _geometries)
    {
        geometriesVK.emplace_back(geometry->getGeometry());
        count += geometry->getCount();
    }

    _geometryInfo =
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,   // sType
        nullptr,                                                            // pNext
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,                    // type
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,          // flag
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,                     // mode
        nullptr,                                                            // srcAccelerationStructure
        nullptr,                                                            // dstAccelerationStructure
        static_cast<uint32_t>(geometriesVK.size()),                         // geometryCount
        geometriesVK.data(),                                                // pGeometries
        nullptr,                                                            // ppGeometries
        { 0 }                                                               // scratchData
    };

    _buildInfo =
    {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
    };
    
    context.getDevice().vkGetAccelerationStructureBuildSizesKHR(context.getDevice().getInstance(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                                &_geometryInfo, &count, &_buildInfo);

    MOUCA_POST_CONDITION(!isNull());
}

void AccelerationBuildGeometry::addGeometry(AccelerationStructureGeometryUPtr&& geometry)
{
    _geometries.emplace_back(std::move(geometry));
}

}