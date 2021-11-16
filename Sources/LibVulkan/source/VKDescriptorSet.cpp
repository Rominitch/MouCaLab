/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKAccelerationStructure.h"
#include "LibVulkan/include/VKDescriptorSet.h"
#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKImage.h"
#include "LibVulkan/include/VKMesh.h"
#include "LibVulkan/include/VKSampler.h"

namespace Vulkan
{

DescriptorPool::DescriptorPool():
_pool(VK_NULL_HANDLE)
{}

void DescriptorPool::initialize(const Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes, const uint32_t maxSets)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());

    const VkDescriptorPoolCreateInfo info =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,      // VkStructureType                sType;
        nullptr,                                            // const void*                    pNext;
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,  // VkDescriptorPoolCreateFlags    flags;
        maxSets,                                            // uint32_t                       maxSets;
        static_cast<uint32_t>(poolSizes.size()),            // uint32_t                       poolSizeCount;
        poolSizes.data()                                    // const VkDescriptorPoolSize*    pPoolSizes;
    };

    if(vkCreateDescriptorPool(device.getInstance(), &info, nullptr, &_pool) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "DescriptorPoolCreationError"));
    }
    MouCa::postCondition(!isNull());
}

void DescriptorPool::release(const Device& device)
{
    MouCa::assertion(!isNull());
    MouCa::assertion(!device.isNull());

    vkDestroyDescriptorPool(device.getInstance(), _pool, nullptr);
    _pool = VK_NULL_HANDLE;
}

DescriptorSetLayout::DescriptorSetLayout():
_layout(VK_NULL_HANDLE)
{
    MouCa::assertion(isNull());
}

void DescriptorSetLayout::addBinding(const VkDescriptorType type, const uint32_t count, const VkShaderStageFlags stageFlags)
{
    MouCa::assertion(isNull()); //DEV Issue: Already call initialize()

    const VkDescriptorSetLayoutBinding descriptorSetLayoutCreateInfo =
    {
        static_cast<uint32_t>(_bindings.size()), // uint32_t              binding;
        type,                                    // VkDescriptorType      descriptorType;
        count,                                   // uint32_t              descriptorCount;
        stageFlags,                              // VkShaderStageFlags    stageFlags;
        nullptr                                  // const VkSampler*      pImmutableSamplers;
    };

    _bindings.emplace_back(descriptorSetLayoutCreateInfo);
}

void DescriptorSetLayout::addUniformBinding(const VkShaderStageFlags stageFlags)
{
    MouCa::assertion(isNull());

    const VkDescriptorSetLayoutBinding descriptorSetLayoutCreateInfo =
    {
        static_cast<uint32_t>(_bindings.size()), // uint32_t              binding;
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       // VkDescriptorType      descriptorType;
        1,                                       // uint32_t              descriptorCount;
        stageFlags,                              // VkShaderStageFlags    stageFlags;
        nullptr                                  // const VkSampler*      pImmutableSamplers;
    };

    _bindings.push_back(descriptorSetLayoutCreateInfo);
}

void DescriptorSetLayout::addImageSamplerBinding(const VkShaderStageFlags stageFlags)
{
    MouCa::assertion(isNull());

    const VkDescriptorSetLayoutBinding descriptorSetLayoutCreateInfo =
    {
        static_cast<uint32_t>(_bindings.size()),   // uint32_t              binding;
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // VkDescriptorType      descriptorType;
        1,                                         // uint32_t              descriptorCount;
        stageFlags,                                // VkShaderStageFlags    stageFlags;
        nullptr                                    // const VkSampler*      pImmutableSamplers;
    };

    _bindings.push_back(descriptorSetLayoutCreateInfo);
}

void DescriptorSetLayout::initialize(const Device& device)
{
    MouCa::assertion(isNull());
    MouCa::assertion(!device.isNull());
    MouCa::assertion(!_bindings.empty()); // DEV Issue: Missing binding !

    const VkDescriptorSetLayoutCreateInfo info =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, // VkStructureType                        sType;
        nullptr,                                             // const void*                            pNext;
        0,                                                   // VkDescriptorSetLayoutCreateFlags       flags;
        static_cast<uint32_t>(_bindings.size()),             // uint32_t                               bindingCount;
        _bindings.data()                                     // const VkDescriptorSetLayoutBinding*    pBindings;
    };

    if(vkCreateDescriptorSetLayout(device.getInstance(), &info, nullptr, &_layout) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "DescriptorLayoutCreationError"));
    }
    MouCa::assertion(!isNull());
}

void DescriptorSetLayout::release(const Device& device)
{
    MouCa::assertion(!isNull());
    MouCa::assertion(!device.isNull());

    vkDestroyDescriptorSetLayout(device.getInstance(), _layout, nullptr);
    _layout = VK_NULL_HANDLE;
    
    _bindings.clear();
    MouCa::assertion(isNull());
}

WriteDescriptorSet::WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, std::vector<VkDescriptorImageInfo>&& vkImageInfo):
_dstBinding(dstBinding), _type(type), _vkImageInfo(std::move(vkImageInfo))
{}

WriteDescriptorSet::WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, std::vector<VkDescriptorBufferInfo>&& bufferInfo) :
_dstBinding(dstBinding), _type(type), _vkBufferInfo(std::move(bufferInfo))
{}

WriteDescriptorSet::WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, DescriptorImageInfos&& imageInfo):
_dstBinding(dstBinding), _type(type), _imageInfo(std::move(imageInfo))
{}

WriteDescriptorSet::WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, DescriptorBufferInfos&& bufferInfo):
_dstBinding(dstBinding), _type(type), _bufferInfo(std::move(bufferInfo))
{}

WriteDescriptorSet::WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, std::vector<VkBufferView>&& bufferView):
_dstBinding(dstBinding), _type(type), _vkBufferView(std::move(bufferView))
{}

WriteDescriptorSet::WriteDescriptorSet(const uint32_t dstBinding, DescriptorAcceleration&& acceleration):
_dstBinding(dstBinding), _type(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR), _acceleration(std::move(acceleration))
{}

VkWriteDescriptorSet WriteDescriptorSet::compute(const DescriptorSet& set, const uint32_t setId)
{
    MouCa::preCondition(!set.isNull());
    MouCa::preCondition(setId < set.getDescriptorSets().size());

    if(_type != VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
    {
        // Compute local vulkan array
        if(!_imageInfo.empty())
        {
            _vkImageInfo.resize(_imageInfo.size());
            std::transform(_imageInfo.cbegin(), _imageInfo.cend(), _vkImageInfo.begin(),
                           [&](const auto& info) -> VkDescriptorImageInfo
                           {
                               return { info._sampler.lock()->getInstance(),
                                        info._imageView.lock()->getInstance(),
                                        info._imageLayout };
                           });
        }
        if(!_bufferInfo.empty())
        {
            _vkBufferInfo.resize(_bufferInfo.size());
            std::transform(_bufferInfo.cbegin(), _bufferInfo.cend(), _vkBufferInfo.begin(),
                           [&](const auto& info) -> VkDescriptorBufferInfo
                           {
                               return { info._buffer.lock()->getBuffer(),
                                        info._offset,
                                        info._range };
                           });
        }

        return
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,   // VkStructureType                  sType;
            nullptr,                                  // const void*                      pNext;
            set.getDescriptorSets()[setId],           // VkDescriptorSet                  dstSet;
            _dstBinding,                              // uint32_t                         dstBinding;
            0,                                        // uint32_t                         dstArrayElement;
            static_cast<uint32_t>(_vkImageInfo.size() + _vkBufferInfo.size() + _vkBufferView.size()),    // uint32_t                         descriptorCount;
            _type,                                                          // VkDescriptorType                 descriptorType;
            _vkImageInfo.empty() ? nullptr : _vkImageInfo.data(),             // const VkDescriptorImageInfo*     pImageInfo;
            _vkBufferInfo.empty() ? nullptr : _vkBufferInfo.data(),            // const VkDescriptorBufferInfo*    pBufferInfo;
            _vkBufferView.empty() ? nullptr : _vkBufferView.data()             // const VkBufferView*              pTexelBufferView;
        };
    }
    else
    {
        _vkAccelerationStruct.resize(_acceleration._structures.size());
        std::transform(_acceleration._structures.cbegin(), _acceleration._structures.cend(), _vkAccelerationStruct.begin(),
                       [&](const auto& info) -> VkAccelerationStructureKHR
                       {
                           return info.lock()->getHandle();
                       });

        _accelerationStructure =
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
            nullptr,
            static_cast<uint32_t>(_vkAccelerationStruct.size()),
            _vkAccelerationStruct.data(),
        };

        return
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,   // VkStructureType                  sType;
            &_accelerationStructure,                  // const void*                      pNext;
            set.getDescriptorSets()[setId],           // VkDescriptorSet                  dstSet;
            _dstBinding,                              // uint32_t                         dstBinding;
            0,                                        // uint32_t                         dstArrayElement;
            static_cast<uint32_t>(_vkAccelerationStruct.size()),    // uint32_t                         descriptorCount;
            _type,                                    // VkDescriptorType                 descriptorType;
            nullptr,                                  // const VkDescriptorImageInfo*     pImageInfo;
            nullptr,                                  // const VkDescriptorBufferInfo*    pBufferInfo;
            nullptr                                   // const VkBufferView*              pTexelBufferView;
        };
    }
}

void DescriptorSet::initialize(const Device& device, const DescriptorPoolWPtr& descriptorPool, const std::vector<VkDescriptorSetLayout>& layouts)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!descriptorPool.expired() && !descriptorPool.lock()->isNull());
    MouCa::preCondition(!layouts.empty());

    // Resize array of descriptors
    //_descriptors.resize(1);// layouts.size());
    _descriptors.resize(layouts.size());
    _pool = descriptorPool;

    const VkDescriptorSetAllocateInfo allocateInfo
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,                     // VkStructureType                 sType;
        nullptr,                                                            // const void*                     pNext;
        _pool.lock()->getInstance(),                                        // VkDescriptorPool                descriptorPool;
        static_cast<uint32_t>(_descriptors.size()),                         // uint32_t                        descriptorSetCount;
        layouts.data()                                                      // const VkDescriptorSetLayout*    pSetLayouts;
    };

    //Generate descriptor
    const VkResult result = vkAllocateDescriptorSets(device.getInstance(), &allocateInfo, _descriptors.data());
    if(result != VK_SUCCESS)
    {
        _descriptors.clear();
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        {
            throw Core::Exception(Core::ErrorData("Vulkan", "PoolDescriptorSetAllocateError"));
        }   
        else
        {
            throw Core::Exception(Core::ErrorData("Vulkan", "DescriptorSetAllocateError"));
        }
    }

    MouCa::postCondition(!isNull());
#ifndef NDEBUG
    for (const auto desc : _descriptors)
    {
        MouCa::preCondition(desc != VK_NULL_HANDLE);
    }
#endif
}

void DescriptorSet::update(const Device& device, const uint32_t setId, std::vector<WriteDescriptorSet>&& writeDescriptor)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());

    _writeDescriptor = std::move(writeDescriptor);
    _setId           = setId;

    update(device);
}

void DescriptorSet::update(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!_writeDescriptor.empty());

    std::vector<VkWriteDescriptorSet> writeSets;
    writeSets.resize(_writeDescriptor.size());

    auto itWrite = writeSets.begin();
    for (auto& write : _writeDescriptor)
    {
        *itWrite = write.compute(*this, _setId);
        ++itWrite;
    }
//     std::transform(_writeDescriptor.cbegin(), _writeDescriptor.cend(), writeSets.begin(),
//         [&](auto& write) -> VkWriteDescriptorSet
//         {
//             return write.compute(*this, _setId);
//         });

    vkUpdateDescriptorSets(device.getInstance(), static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
}

void DescriptorSet::release(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!_pool.expired());

    if(vkFreeDescriptorSets(device.getInstance(), _pool.lock()->getInstance(), static_cast<uint32_t>(_descriptors.size()), _descriptors.data()) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "DescriptorCreationError"));
    }
    _descriptors.clear();
    _pool.reset();

    MouCa::assertion(isNull());
}

}