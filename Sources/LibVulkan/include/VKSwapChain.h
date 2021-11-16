/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;
    class Surface;
    class SurfaceFormat;

    class Sequence;
    using SequenceWPtr = std::weak_ptr<Sequence>;

    class SwapChainImage final
    {
        private:
            VkImage     _image;
            VkImageView _view;

            MOUCA_NOCOPY(SwapChainImage);

        public:
            /// Constructor
            SwapChainImage();
            /// Destructor
            ~SwapChainImage();
            
            SwapChainImage(SwapChainImage&&) = default;
            SwapChainImage& operator=(SwapChainImage&&) = default;

            void initialize(const Device& device, const VkFormat& format, const VkImage& image);
            void release(const Device& device);
            
            const VkImage& getImage() const
            {
                return _image;
            }

            const VkImageView& getView() const
            {
                return _view;
            }

            //------------------------------------------------------------------------
            /// \brief  Check if device is null.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const
            {
                return _image == VK_NULL_HANDLE || _view == VK_NULL_HANDLE;
            }
    };

    //----------------------------------------------------------------------------
    /// \brief Allow to create swapchain system based on surface and this format.
    class SwapChain final
    {
        MOUCA_NOCOPY(SwapChain);

        public:
            /// Constructor
            SwapChain();
            /// Destructor
            ~SwapChain();

            void initialize(const Device& device, const Surface& surface, const SurfaceFormat& format);

            void release(const Device& device);

            void generateImages(const Device& device, const SurfaceFormat& format);

            const VkSwapchainKHR& getInstance() const                { return _swapChain; }
            const std::vector<SwapChainImage>& getImages() const     { return _images; }

            void        setCurrentImage(const uint32_t currentImage)
            {
                MouCa::preCondition( currentImage < _images.size() ); //DEV Issue: Out of range !
                _currentImage = currentImage;
            }
            uint32_t    getCurrentImage() const                      { return _currentImage; }

            //------------------------------------------------------------------------
            /// \brief  Check if device is null.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const
            {
                return _swapChain == VK_NULL_HANDLE;
            }

            bool isReady() const       { return _ready; }
            void setReady(bool ready)  { _ready = ready; }

            void registerSequence(SequenceWPtr sequence);
            void unregisterSequence(SequenceWPtr sequence);
            
        private:
            VkSwapchainKHR                  _swapChain;     ///< Instance of swap-chain on Vulkan.
            std::vector<SwapChainImage>     _images;        ///< All associated images to swap-chain.
            uint32_t                        _currentImage;

            bool                            _ready;
            std::vector<SequenceWPtr>       _linkSequences;
    };
}