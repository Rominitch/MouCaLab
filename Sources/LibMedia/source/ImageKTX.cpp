#include "Dependencies.h"

#include <LibMedia/include/ImageKTX.h>

namespace Media
{

void ImageKTX::initialize(const Core::Path& path)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!path.empty());

    // Create Gli image
    _glImage = std::make_unique<gli::texture>(gli::load(path.string()));

    MOUCA_PRE_CONDITION(!isNull());
}

void ImageKTX::createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    MOUCA_THROW_ERROR(u8"BasicError", u8"ImageNoneImplemented");
}

void ImageKTX::release()
{
    MOUCA_PRE_CONDITION(!isNull());

    _glImage.reset();

    MOUCA_POST_CONDITION(isNull());
}

uint32_t ImageKTX::getLayers() const
{
    return static_cast<uint32_t>(_glImage->layers());
}

uint32_t ImageKTX::getLevels() const
{
    return static_cast<uint32_t>(_glImage->levels());
}

RT::Array3ui ImageKTX::getExtents(const uint32_t level) const
{
    const auto ext = _glImage->extent(level);
    return {ext.x, ext.y, ext.z };
}

void const* const ImageKTX::getRAWData(const uint32_t layer, const uint32_t level) const
{
    return _glImage->data(layer, 0, level);
}

size_t ImageKTX::getMemoryOffset(const uint32_t layer, const uint32_t level) const
{
    size_t offset = 0;

    for (uint32_t layerId = 0; layerId < getLayers(); layerId++)
    {
        for (uint32_t mipLevel = 0; mipLevel < getLevels(); mipLevel++)
        {
            if(layer == layerId && mipLevel == level)
                return offset;
            offset += _glImage->size(mipLevel);
        }
    }

    return offset;
}

size_t ImageKTX::getMemorySize() const
{
    return _glImage->size();
}

void ImageKTX::saveImage(const Core::Path& filename)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!filename.empty());

    if( !gli::save(*_glImage, filename.string()) )
    {
        MOUCA_THROW_ERROR_1(u8"BasicError", u8"ImageKTXSave", filename.u8string());
    }
}

bool ImageKTX::compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                       size_t* nbDefectPixels, double* distance4D) const
{
    MOUCA_THROW_ERROR(u8"BasicError", u8"ImageNoneImplemented");
}

ImageKTX::Target ImageKTX::getTarget() const
{
    static const std::array<Image::Target, gli::TARGET_COUNT> convert =
    {
        Target::Type1D, Target::Type1DArray, Target::Type2D, Target::Type2DArray, Target::Type3D, Target::TypeUnknown, Target::TypeUnknown, Target::TypeUnknown, Target::TypeUnknown
    };

    return convert[_glImage->target()];
}

}