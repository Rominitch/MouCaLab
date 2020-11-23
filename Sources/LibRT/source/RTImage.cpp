/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibRT/include/RTImage.h>

#include <LibRT/include/RTBufferCPU.h>

namespace RT
{

void ImageLinkedCPU::initialize(const Target target, const RT::BufferCPUBaseWPtr& imageBuffer, const RT::Array3ui& extents)
{
    BT_PRE_CONDITION(isNull());                 //DEV Issue: Need none initialized image.
    BT_PRE_CONDITION(!imageBuffer.expired());   //DEV Issue: Need valid data.
    BT_PRE_CONDITION(extents.x >= 1 && extents.y >= 1 && extents.z >= 1); //DEV Issue: Need valid size.

    _target       = target;
    _linkedBuffer = imageBuffer;
    _extents      = extents;

    BT_POST_CONDITION(!isNull()); //DEV Issue: Ready to work.
}

void ImageLinkedCPU::release()
{
    BT_PRE_CONDITION(!isNull()); //DEV Issue: Not initialize ?

    _linkedBuffer.reset();
    _extents = { 0,0,0 };

    BT_POST_CONDITION(isNull()); //DEV Issue: Ready to work.
}

void ImageLinkedCPU::createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    BT_THROW_ERROR(u8"BasicError", u8"ImageNoneImplemented");
}

void ImageLinkedCPU::saveImage(const Core::Path& filename)
{
    BT_THROW_ERROR(u8"BasicError", u8"ImageNoneImplemented");
}

bool ImageLinkedCPU::compare(const Image & reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                             size_t * nbDefectPixels, double* distance4D) const
{
    BT_THROW_ERROR(u8"BasicError", u8"ImageNoneImplemented");
}

void const* const ImageLinkedCPU::getRAWData(const uint32_t layer, const uint32_t level) const
{
    BT_PRE_CONDITION(!isNull()); //DEV Issue: Ready to work.

    return _linkedBuffer.lock()->getData();
}

size_t ImageLinkedCPU::getMemoryOffset(const uint32_t layer, const uint32_t level) const
{
    BT_PRE_CONDITION(!isNull()); //DEV Issue: Ready to work.
    BT_PRE_CONDITION(layer < getLayers());
    BT_PRE_CONDITION(level < getLevels());

    return 0;
}

size_t ImageLinkedCPU::getMemorySize() const
{
    BT_PRE_CONDITION(!isNull()); //DEV Issue: Ready to work.

    return _linkedBuffer.lock()->getByteSize();
}

bool ImageLinkedCPU::isNull() const
{
    return _linkedBuffer.expired();
}

}