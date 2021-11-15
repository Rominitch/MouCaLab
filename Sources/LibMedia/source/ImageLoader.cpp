/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibRT/include/RTBufferCPU.h>
#include <LibRT/include/RTBufferDescriptor.h>

#include <LibMedia/include/ImageFI.h>
#include <LibMedia/include/ImageKTX.h>

#include <LibMedia/include/ImageLoader.h>

namespace Media
{

RT::ImageSPtr ImageLoader::openImage(const Core::Path& fileName)
{
    MOUCA_PRE_CONDITION( !fileName.empty() );

    const auto ext = fileName.extension().u8string();
    // Special OpenGL/Vulkan format
    if(ext == u8".dds" || ext == u8".ktx" || ext == u8".kmg")
    {
        auto image = std::make_shared<ImageKTX>();
        image->initialize(fileName);
        return image;
    }

    // All formats (PNG, JPG, ...)
    auto image = std::make_shared<ImageFI>();
    image->initialize(fileName);
    return image;
}

RT::ImageSPtr ImageLoader::createImageFI(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    auto image =std::make_shared<ImageFI>();
    image->createFill(imageBuffer, width, height);
    return image;
}

void ImageLoader::export2D(RT::Image& image, const Core::Path& fileName)
{
    const auto ext = fileName.extension().u8string();
    if (ext == u8".dds" || ext == u8".ktx" || ext == u8".kmg")
    {
        MOUCA_ASSERT_HEADER(false, "Not implemented");
    }
    else
    {
        auto ktxImage = dynamic_cast<Media::ImageKTX*>(&image);
        if(ktxImage != nullptr)
        {
            const auto layer = 0;
            const auto level = 0;
            
            const auto extents = ktxImage->getExtents(level);
            RT::BufferLinkedCPU buffer;
            buffer.create(RT::BufferDescriptor(ktxImage->getMemorySize()), static_cast<size_t>(extents.x*extents.y), ktxImage->getRAWData(layer, level));

            ImageFI fiImage;
            fiImage.createFill(buffer, extents.x, extents.y);
            fiImage.saveImage(fileName);
        }
        else
        {
            image.saveImage(fileName);
        }
    }
}

}