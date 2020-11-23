#pragma once


namespace RT
{
    class BufferCPUBase;

    class Image;
    using ImageSPtr = std::shared_ptr<Image>;
}

namespace Media
{
    class ImageLoader
    {
        public:
            RT::ImageSPtr openImage(const Core::Path& strFilename);

            RT::ImageSPtr createImageFI(const RT::BufferCPUBase& ImageBuffer, const uint32_t szWidth, const uint32_t szHeight);
    };
}