/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Buffer;
    using BufferWPtr = std::weak_ptr<Buffer>;

    class Device;

    class Sampler;
    using SamplerWPtr = std::weak_ptr<Sampler>;

    class ImageView;
    using ImageViewWPtr = std::weak_ptr<ImageView>;

    class DescriptorPool
    {
        private:
            VkDescriptorPool _pool;

            MOUCA_NOCOPY(DescriptorPool);

        public:
            DescriptorPool();

            ~DescriptorPool()
            {
                MOUCA_ASSERT(isNull());
            }

            void initialize(const Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes, const uint32_t maxSets);

            void release(const Device& device);

            const VkDescriptorPool& getInstance() const
            {
                return _pool;
            }

            bool isNull() const
            {
                return _pool == VK_NULL_HANDLE;
            }
    };
    using DescriptorPoolSPtr = std::shared_ptr<DescriptorPool>;
    using DescriptorPoolWPtr = std::weak_ptr<DescriptorPool>;

    class DescriptorSetLayout final
    {
        private:
            VkDescriptorSetLayout                       _layout;
            std::vector<VkDescriptorSetLayoutBinding>   _bindings;

            MOUCA_NOCOPY(DescriptorSetLayout);

        public:
            DescriptorSetLayout();
            DescriptorSetLayout(DescriptorSetLayout&&) = default;

            ~DescriptorSetLayout()
            {
                MOUCA_ASSERT(isNull());
            }

            void addBinding(const VkDescriptorType type, const uint32_t count, const VkShaderStageFlags stageFlags);

            [[deprecated]] void addUniformBinding(const VkShaderStageFlags stageFlags);
            
            [[deprecated]] void addImageSamplerBinding(const VkShaderStageFlags stageFlags);

            void initialize(const Device& device);

            void release(const Device& device);

            const VkDescriptorSetLayout& getInstance() const
            {
                return _layout;
            }

            bool isNull() const
            {
                return _layout == VK_NULL_HANDLE;
            }
    };

    class DescriptorSet;

    struct DescriptorImageInfo
    {
        SamplerWPtr      _sampler;
        ImageViewWPtr    _imageView;
        VkImageLayout    _imageLayout;
    };
    using DescriptorImageInfos = std::vector<DescriptorImageInfo>;

    struct DescriptorBufferInfo
    {
        BufferWPtr      _buffer;
        VkDeviceSize    _offset;
        VkDeviceSize    _range;
    };
    using DescriptorBufferInfos = std::vector<DescriptorBufferInfo>;

    class WriteDescriptorSet final
    {
        public:
            WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, DescriptorImageInfos&&  imageInfo);
            WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, DescriptorBufferInfos&& bufferInfo);

            [[deprecated]] WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, std::vector<VkDescriptorImageInfo>&& vkImageInfo);
            [[deprecated]] WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, std::vector<VkDescriptorBufferInfo>&& bufferInfo);
            [[deprecated]] WriteDescriptorSet(const uint32_t dstBinding, const VkDescriptorType type, std::vector<VkBufferView>&& bufferView);

            VkWriteDescriptorSet compute(const DescriptorSet& set, const uint32_t setId);

        private:
            const uint32_t                         _dstBinding;
            const VkDescriptorType                 _type;
            const DescriptorImageInfos             _imageInfo;
            const DescriptorBufferInfos            _bufferInfo;

            std::vector<VkDescriptorImageInfo>     _vkImageInfo;
            std::vector<VkDescriptorBufferInfo>    _vkBufferInfo;
            std::vector<VkBufferView>              _vkBufferView;
    };

    class DescriptorSet
    {
        MOUCA_NOCOPY(DescriptorSet);

        public:
            DescriptorSet() = default;

            ~DescriptorSet()
            {
                MOUCA_POST_CONDITION(isNull());
            }

            void initialize(const Device& device, const DescriptorPoolWPtr& descriptorPool, const std::vector<VkDescriptorSetLayout>& layouts);

            void release(const Device& device);

            const std::vector<VkDescriptorSet>& getDescriptorSets() const
            {
                return _descriptors;
            }

            const VkDescriptorSet& getInstance(const size_t id) const
            {
                MOUCA_PRE_CONDITION(id < _descriptors.size());
                return _descriptors[id];
            }

            bool isNull() const
            {
                return _descriptors.empty();
            }

            void update(const Device& device, const uint32_t setId, std::vector<WriteDescriptorSet>&& writeDescriptor);

            void update(const Device& device);

        private:
            uint32_t                        _setId;
            std::vector<WriteDescriptorSet> _writeDescriptor;

            std::vector<VkDescriptorSet> _descriptors;
            DescriptorPoolWPtr           _pool;
    };

    
}