/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Abstraction of Texture/Buffer on GPU to allow generic storage. 
    ///
    /// \see   Vulkan::Image
    class ImageGPU
    {
        MOUCA_NOCOPY_NOMOVE(ImageGPU);

        public:
            virtual ~ImageGPU() = default;

        protected:
            ImageGPU() = default;
    };

    using ImageGPUSPtr = std::shared_ptr<ImageGPU>;
}