/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTImageGPU.h>

#include <LibVulkan/include/VKBuffer.h>

namespace RT
{
    class Image;
}

namespace Vulkan
{
    // Forward description
    class Device;

    class ImageView;
    using ImageViewUPtr = std::unique_ptr<ImageView>;
    using ImageViewSPtr = std::shared_ptr<ImageView>;
    using ImageViewWPtr = std::weak_ptr<ImageView>;

    class Sampler;

    /// Vulkan Image
    class Image final : public RT::ImageGPU
    {
        MOUCA_NOCOPY_NOMOVE(Image);

        public:
            /// Define all sizes of texture
            struct Size
            {
                Size(const VkExtent3D& extent={ 1,1,1 }, const uint32_t mipLevels=1, const uint32_t arrayLayers=1):
                _extent(extent), _mipLevels(mipLevels), _arrayLayers(arrayLayers)
                {}

                bool isValid() const { return _extent.width > 0 && _extent.height > 0 && _extent.depth > 0 && _mipLevels > 0 && _arrayLayers > 0; }

                VkExtent3D _extent;
                uint32_t   _mipLevels;
                uint32_t   _arrayLayers;
            };

            Image();
            ~Image();

            void initialize(const Device& device,
                            const Size& size,
                            const VkImageType imageType, const VkFormat format,
                            const VkSampleCountFlagBits sampleCount, const VkImageTiling tiling, const VkImageUsageFlags imageUsageFlags, const VkSharingMode sharingMode, const VkImageLayout initialLayout,
                            const VkMemoryPropertyFlags memoryProperty);

            void release(const Device& device, const bool removeView=true);

            void resize(const Device& device, const VkExtent3D& newSize);

            bool isNull() const { return _image == VK_NULL_HANDLE && _memory.isNull(); }

            const VkImage&          getImage() const        { return _image; }
            VkFormat                getFormat() const       { return _imageCreateInfo.format; }
            VkImageType             getType() const         { return _imageCreateInfo.imageType; }
            MemoryImage&            getMemory()             { return _memory; }
            const VkExtent3D&       getExtent() const       { return _imageCreateInfo.extent; }
            const uint32_t&         getArraySize() const    { return _imageCreateInfo.arrayLayers; }
            VkSampleCountFlagBits   getSamples() const      { return _imageCreateInfo.samples; }
            uint32_t                getMaxMipLevels() const { return _imageCreateInfo.mipLevels; }

            void createSampler(const Device& device, Sampler& sampler) const;

            void readSubresourceLayout(const Device& device, const VkImageSubresource& subResource, VkSubresourceLayout& subResourceLayout) const;

            ImageViewWPtr createView(const Device& device, const VkImageViewType viewType, const VkFormat format, const VkComponentMapping& components, const VkImageSubresourceRange& subresourceRange);
            void removeView(const Device& device, ImageViewWPtr view);

        private:
            VkImageCreateInfo   _imageCreateInfo;   ///< Building info (to auto rebuild)

            // Final info
            VkImage                 _image;         ///< Image
            MemoryImage             _memory;        ///< Memory area

            /// Views
            std::vector<ImageViewSPtr> _views;  ///< [OWNERSHIP] All view based to current image.
    };

    using ImageSPtr = std::shared_ptr<Image>;
    using ImageWPtr = std::weak_ptr<Image>;
    using Images    = std::vector<ImageSPtr>;

    class ImageView
    {
        MOUCA_NOCOPY_NOMOVE(ImageView);

        public:
            ImageView();
            ~ImageView();

            bool isNull() const { return _view == VK_NULL_HANDLE; }

            /// \brief Initialize View from image.
            /// \param[in] format : New format exposed into shader (use VK_FORMAT_UNDEFINED to copy image format automatically)
            void initialize(const Device& device, const Image& image,
                            const VkImageViewType viewType, const VkFormat format,
                            const VkComponentMapping& components, const VkImageSubresourceRange& subresourceRange);

            void release(const Device& device);

            const VkImageView& getInstance() const  { return _view; }
            const Image&       getImage() const     { return *_image; }

            const VkImageViewCreateInfo& getInfo() const { return _viewInfo; }
        private:
            const Image*    _image;     ///< Link to parent.

            VkImageView             _view;      ///< Handle to view
            VkImageViewCreateInfo   _viewInfo;  ///< Current creation info.
    };
}