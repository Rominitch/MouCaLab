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
            static RT::ImageSPtr openImage(const Core::Path& strFilename);

            static RT::ImageSPtr createImageFI(const RT::BufferCPUBase& ImageBuffer, const uint32_t szWidth, const uint32_t szHeight);

            static void export2D(RT::Image& image, const Core::Path& fileName);
    };
}