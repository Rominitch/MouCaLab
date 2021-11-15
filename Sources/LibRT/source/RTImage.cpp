/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibRT/include/RTImage.h>

#include <LibRT/include/RTBufferCPU.h>

namespace RT
{

void ImageLinkedCPU::initialize(const Target target, const RT::BufferCPUBaseWPtr& imageBuffer, const RT::Array3ui& extents)
{
    MOUCA_PRE_CONDITION(isNull());                 //DEV Issue: Need none initialized image.
    MOUCA_PRE_CONDITION(!imageBuffer.expired());   //DEV Issue: Need valid data.
    MOUCA_PRE_CONDITION(extents.x >= 1 && extents.y >= 1 && extents.z >= 1); //DEV Issue: Need valid size.

    _target       = target;
    _linkedBuffer = imageBuffer;
    _extents      = extents;

    MOUCA_POST_CONDITION(!isNull()); //DEV Issue: Ready to work.
}

void ImageLinkedCPU::release()
{
    MOUCA_PRE_CONDITION(!isNull()); //DEV Issue: Not initialize ?

    _linkedBuffer.reset();
    _extents = { 0,0,0 };

    MOUCA_POST_CONDITION(isNull()); //DEV Issue: Ready to work.
}

void ImageLinkedCPU::createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    MOUCA_UNUSED(imageBuffer);
    MOUCA_UNUSED(width);
    MOUCA_UNUSED(height);
    MOUCA_THROW_ERROR("BasicError", "ImageNoneImplemented");
}

void ImageLinkedCPU::saveImage(const Core::Path& filename)
{
    MOUCA_UNUSED(filename);
    MOUCA_THROW_ERROR("BasicError", "ImageNoneImplemented");
}

void ImageLinkedCPU::export2D(const Core::Path& filename)
{
    MOUCA_UNUSED(filename);
    MOUCA_THROW_ERROR("BasicError", "ImageNoneImplemented");
}

bool ImageLinkedCPU::compare(const Image & reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                             size_t * nbDefectPixels, double* distance4D) const
{
    MOUCA_UNUSED(reference);
    MOUCA_UNUSED(nbMaxDefectPixels);
    MOUCA_UNUSED(maxDistance4D);
    MOUCA_UNUSED(nbDefectPixels);
    MOUCA_UNUSED(distance4D);
    MOUCA_THROW_ERROR("BasicError", "ImageNoneImplemented");
}

const RT::Image::HandlerMemory ImageLinkedCPU::getRAWData(const uint32_t layer, const uint32_t level) const
{
    MOUCA_PRE_CONDITION(!isNull()); //DEV Issue: Ready to work.

    MOUCA_UNUSED(layer);
    MOUCA_UNUSED(level);

    return _linkedBuffer.lock()->getData();
}

size_t ImageLinkedCPU::getMemoryOffset(const uint32_t layer, const uint32_t level) const
{
    MOUCA_PRE_CONDITION(!isNull()); //DEV Issue: Ready to work.
    MOUCA_PRE_CONDITION(layer < getLayers());
    MOUCA_PRE_CONDITION(level < getLevels());

    MOUCA_UNUSED(layer);
    MOUCA_UNUSED(level);
    return 0;
}

size_t ImageLinkedCPU::getMemorySize() const
{
    MOUCA_PRE_CONDITION(!isNull()); //DEV Issue: Ready to work.

    return _linkedBuffer.lock()->getByteSize();
}

bool ImageLinkedCPU::isNull() const
{
    return _linkedBuffer.expired();
}

}