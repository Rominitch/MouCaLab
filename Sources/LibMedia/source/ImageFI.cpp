#include "Dependencies.h"

#include "LibMedia/include/ImageFI.h"

#include "LibMedia/include/ImageLoader.h"

#include "LibRT/include/RTBufferCPU.h"
#include "LibRT/include/RTBufferDescriptor.h"

namespace Media
{

void ImageFI::initialize(const Core::Path& path)
{
    MouCa::preCondition(isNull());
    
    //Get file format
    FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeU(path.c_str(), 0);
    if (imageFormat == FIF_UNKNOWN)
    {
        //Check extension file
        imageFormat = FreeImage_GetFIFFromFilenameU(path.c_str());
        if (imageFormat == FIF_UNKNOWN)
        {
            throw Core::Exception(Core::ErrorData("ModuleError", "FIUnknownFileError") << path.string());
        }
    }

    //Check that the plugin has reading capabilities ...
    if (FreeImage_FIFSupportsReading(imageFormat))
    {
        FIBITMAP* imageData = FreeImage_LoadU(imageFormat, path.c_str(), 0);
        MouCa::assertion(imageData != nullptr); // DEV Issue: file is not exist !
        _imageData = FreeImage_ConvertTo32Bits(imageData);
        FreeImage_Unload(imageData);
    }
    else
    {
        throw Core::Exception(Core::ErrorData("ModuleError", "FIReadFileError") << path.string());
    }

    MouCa::preCondition(!isNull());
}

void ImageFI::createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    MouCa::preCondition( isNull() );
    MouCa::preCondition(imageBuffer.getData() != nullptr);
    MouCa::preCondition(width > 0 && height > 0);
    MouCa::preCondition(width * height == imageBuffer.getNbElements());

    const RT::BufferDescriptor& descriptor = imageBuffer.getDescriptor();
    MouCa::preCondition(descriptor.getNbDescriptors() == 1 && descriptor.getComponentDescriptor(0).getNbComponents() == 4); //DEV: We support only rgba component

    //Create new buffer
    _imageData = FreeImage_Allocate(static_cast<int>(width), static_cast<int>(height), 8 * static_cast<int>(descriptor.getByteSize()));
    if (_imageData == nullptr)
    {
        throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError") << "_imageData");
    }

    const char* pSource = reinterpret_cast<const char*>(imageBuffer.getData());
    char* pPixel = reinterpret_cast<char*>(FreeImage_GetBits(_imageData));

    //Copy the picture
    const uint64_t pitchRow = imageBuffer.getPitchRow();
    if (pitchRow == 0)
    {
        memcpy(pPixel, pSource, descriptor.getByteSize() * imageBuffer.getNbElements());
    }
    else
    {
        for (uint64_t rowID = 0; rowID < height; ++rowID)
        {
            memcpy(pPixel, pSource, descriptor.getByteSize() * width);
            pSource += pitchRow;
            pPixel += descriptor.getByteSize() * width;
        }
    }

    // Change orientation to Y down
    FreeImage_FlipVertical(_imageData);

    MouCa::postCondition(!isNull());
}

void ImageFI::release()
{
    MouCa::preCondition(!isNull());
    FreeImage_Unload(_imageData);
    _imageData = nullptr;

    MouCa::postCondition(isNull());
}

RT::Array3ui ImageFI::getExtents(const uint32_t level) const
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(level < getLevels());
    return { FreeImage_GetWidth(_imageData), FreeImage_GetHeight(_imageData), 1 };
}

void ImageFI::saveImage(const Core::Path& filename)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!filename.empty());

    //Check we have a picture
    if(_imageData == nullptr)
    {
        throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError") << "m_pImageData");
    }

    //Try to guess the file format from the file extension
    const FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFIFFromFilenameU(filename.c_str());
    if(imageFormat == FIF_UNKNOWN)
    {
        throw Core::Exception(Core::ErrorData("ModuleError", "FIUnknownFileError") << filename.string());
    }

    //Check that the plugin has sufficient writing and export capabilities ...
    const unsigned int bpp = FreeImage_GetBPP(_imageData);
    if(FreeImage_FIFSupportsWriting(imageFormat) && FreeImage_FIFSupportsExportBPP(imageFormat, bpp))
    {
        // ok, we can save the file
        if(FreeImage_SaveU(imageFormat, _imageData, filename.c_str(), 0) == FALSE)
        {
            throw Core::Exception(Core::ErrorData("ModuleError", "FISaveFileError") << filename.string());
        }
    }
    else
    {
        throw Core::Exception(Core::ErrorData("ModuleError", "FISaveFileError") << filename.string());
    }
}

void ImageFI::export2D(const Core::Path& filename)
{
    ImageLoader::export2D(*this, filename);
}

bool ImageFI::compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                      size_t* nbDefectPixels, double* distance) const
{
    MouCa::preCondition(!isNull());

    const uint32_t layer = 0;
    const uint32_t level = 0;
    const RT::Array3ui extents = getExtents(level);
    const bool equal = extents == reference.getExtents(level);
    if(!equal)
    {
#ifndef NDEBUG
        const RT::Array3ui refExtents = reference.getExtents(level);
        Core::String message = Core::String("Comparison image: different size: ") + std::to_string(extents.x) + Core::String("x") + std::to_string(extents.y)
            + Core::String(" != ")
            + std::to_string(refExtents.x) + Core::String("x") + std::to_string(refExtents.y)
            + Core::String("\nComparison: FAILURE");
        MouCa::logConsole(message);
#endif
        return equal;
    }
    Core::ColorUC32 const* const src = getData<Core::ColorUC32>(layer, level);
    Core::ColorUC32 const* const ref = reference.getData<Core::ColorUC32>(layer, level);

    double max = 0;
    size_t nbDefect = 0;
    const size_t memory = extents.x * extents.y * extents.z;
    for(size_t id = 0; id < memory; ++id)
    {
        const double distance4D = fabs(src[id].m_color[0] - ref[id].m_color[0])
            + fabs(src[id].m_color[1] - ref[id].m_color[1])
            + fabs(src[id].m_color[2] - ref[id].m_color[2])
            + fabs(src[id].m_color[3] - ref[id].m_color[3]);
        if(distance4D > maxDistance4D)
        {
            ++nbDefect;
            max = std::max(max, distance4D);
        }
    }

    if (nbDefectPixels != nullptr)
    {
        *nbDefectPixels = nbDefect;
    }
    if (distance != nullptr)
    {
        *distance = max;
    }

#ifndef NDEBUG
    const Core::String state = (nbMaxDefectPixels >= nbDefect) ? Core::String("SUCCESS\n") : Core::String("FAILURE\n");
    Core::String message = Core::String("Comparison image: max defect distance: ") + std::to_string(max) + Core::String(" < ") + std::to_string(maxDistance4D)
        + Core::String("\n                  pixel count: ") + std::to_string(nbDefect) + Core::String(" < ") + std::to_string(nbMaxDefectPixels)
        + Core::String("\nComparison: ") + state;
    MouCa::logConsole(message);
#endif
    return nbMaxDefectPixels >= nbDefect;
}

size_t ImageFI::getMemoryOffset(const uint32_t layer, const uint32_t level) const
{
    MouCa::preCondition(layer < getLayers());
    MouCa::preCondition(level < getLevels());
    
    // Just a single buffer with one image (no layer or mip-mapping)
    return 0;
}

size_t ImageFI::getMemorySize() const
{
    // WARNING: FreeImage use 32 bits memory: sniff !
    return FreeImage_GetMemorySize(_imageData);
}

}