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
    MouCa::preCondition(isNull());                 //DEV Issue: Need none initialized image.
    MouCa::preCondition(!imageBuffer.expired());   //DEV Issue: Need valid data.
    MouCa::preCondition(extents.x >= 1 && extents.y >= 1 && extents.z >= 1); //DEV Issue: Need valid size.

    _target       = target;
    _linkedBuffer = imageBuffer;
    _extents      = extents;

    MouCa::postCondition(!isNull()); //DEV Issue: Ready to work.
}

void ImageLinkedCPU::release()
{
    MouCa::preCondition(!isNull()); //DEV Issue: Not initialize ?

    _linkedBuffer.reset();
    _extents = { 0,0,0 };

    MouCa::postCondition(isNull()); //DEV Issue: Ready to work.
}

void ImageLinkedCPU::createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    MOUCA_UNUSED(imageBuffer);
    MOUCA_UNUSED(width);
    MOUCA_UNUSED(height);
    throw Core::Exception(Core::ErrorData("BasicError", "ImageNoneImplemented"));
}

void ImageLinkedCPU::saveImage(const Core::Path& filename)
{
    MOUCA_UNUSED(filename);
    throw Core::Exception(Core::ErrorData("BasicError", "ImageNoneImplemented"));
}

void ImageLinkedCPU::export2D(const Core::Path& filename)
{
    MOUCA_UNUSED(filename);
    throw Core::Exception(Core::ErrorData("BasicError", "ImageNoneImplemented"));
}

bool ImageLinkedCPU::compare(const Image & reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                             size_t * nbDefectPixels, double* distance4D) const
{
    MOUCA_UNUSED(reference);
    MOUCA_UNUSED(nbMaxDefectPixels);
    MOUCA_UNUSED(maxDistance4D);
    MOUCA_UNUSED(nbDefectPixels);
    MOUCA_UNUSED(distance4D);
    throw Core::Exception(Core::ErrorData("BasicError", "ImageNoneImplemented"));
}

const RT::Image::HandlerMemory ImageLinkedCPU::getRAWData(const uint32_t layer, const uint32_t level) const
{
    MouCa::preCondition(!isNull()); //DEV Issue: Ready to work.

    MOUCA_UNUSED(layer);
    MOUCA_UNUSED(level);

    return _linkedBuffer.lock()->getData();
}

size_t ImageLinkedCPU::getMemoryOffset(const uint32_t layer, const uint32_t level) const
{
    MouCa::preCondition(!isNull()); //DEV Issue: Ready to work.
    MouCa::preCondition(layer < getLayers());
    MouCa::preCondition(level < getLevels());

    MOUCA_UNUSED(layer);
    MOUCA_UNUSED(level);
    return 0;
}

size_t ImageLinkedCPU::getMemorySize() const
{
    MouCa::preCondition(!isNull()); //DEV Issue: Ready to work.

    return _linkedBuffer.lock()->getByteSize();
}

bool ImageLinkedCPU::isNull() const
{
    return _linkedBuffer.expired();
}

}