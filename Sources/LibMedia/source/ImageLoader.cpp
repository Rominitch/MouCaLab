/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

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

    const Core::String ext = fileName.extension().u8string();
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

}