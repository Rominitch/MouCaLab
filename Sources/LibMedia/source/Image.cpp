#include "Dependancies.h"

#include <LibRT/include/RTBufferCPU.h>
#include <LibRT/include/RTBufferDescriptor.h>

#include <LibMedia/include/Image.h>

//Memory Leak Linker
#ifndef NDEBUG
#   ifdef DEBUG_NEW
#       undef DEBUG_NEW
#   endif
#   define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#   define new DEBUG_NEW
#endif

namespace Media
{

void Image::release()
{
    BT_PRE_CONDITION(!isNull());
    FreeImage_Unload(_imageData);
    _imageData = NULL;

    _extents = { 0,0,0 };
    
    _glImage.reset();
    _filename.clear();

    BT_POST_CONDITION(isNull());
}

void Image::openImage(const BT::StringOS& fileName)
{
    BT_PRE_CONDITION(!isLoaded());

    std::filesystem::path fileImage(fileName);
    const BT::String ext = fileImage.extension().u8string();
    // Special OpenGL/Vulkan format
    if(ext == u8".dds" || ext == u8".ktx" || ext == u8".kmg")
    {
        _glImage = std::make_unique<gli::texture>(gli::load(BT::convertToU8(fileName)));
        _extents = { _glImage->extent(0).x, _glImage->extent(0).y, 1 };
    }
    else // All formats (PNG, JPG, ...)
    {
        //Get file format
        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeU(fileName.c_str(), 0);
        if(imageFormat == FIF_UNKNOWN)
        {
            //Check extension file
            imageFormat = FreeImage_GetFIFFromFilenameU(fileName.c_str());
            if(imageFormat == FIF_UNKNOWN)
            {
                BT_THROW_ERROR_1(u8"ModuleError", u8"FIUnknownFileError", BT::convertToU8(fileName));
            }
        }

        //Check that the plugin has reading capabilities ...
        if(FreeImage_FIFSupportsReading(imageFormat))
        {
            FIBITMAP* imageData = FreeImage_LoadU(imageFormat, fileName.c_str(), 0);
            BT_ASSERT(imageData != nullptr); // DEV Issue: file is not exist !
            _imageData = FreeImage_ConvertTo32Bits(imageData);
            FreeImage_Unload(imageData);

            _extents = { FreeImage_GetWidth(_imageData), FreeImage_GetHeight(_imageData), 1 };
        }
        else
        {
            BT_THROW_ERROR_1(u8"ModuleError", u8"FIReadFileError", BT::convertToU8(fileName));
        }
    }
    _filename = fileName;

    BT_POST_CONDITION(isLoaded());
}

void Image::createImage(const RT::BufferCPUBase& imageBuffer, const BT::uint64 width, const BT::uint64 height)
{
    //BT_PRE_CONDITION(!isNull());
    BT_PRE_CONDITION(!isLoaded());

    BT_ASSERT(width * height == imageBuffer.getNbElements());
    const RT::BufferDescriptor& descriptor = imageBuffer.getDescriptor();
    BT_ASSERT(descriptor.getNbDescriptors() == 1 && descriptor.getComponentDescriptor(0).getNbComponents() == 4); //DEV: We support only rgba component

    //Create new buffer
    _imageData = FreeImage_Allocate(static_cast<int>(width), static_cast<int>(height), 8 * static_cast<int>(descriptor.getByteSize()));
    if(_imageData == NULL)
    {
        BT_THROW_ERROR_1(u8"BasicError", u8"NULLPointerError", u8"_imageData");
    }

    _extents = { width, height, 1 };

    const char* pSource = reinterpret_cast<const char*>(imageBuffer.getData());
    char* pPixel = reinterpret_cast<char*>(FreeImage_GetBits(_imageData));

    //Copy the picture
    const BT::uint64 pitchRow = imageBuffer.getPitchRow();
    if(pitchRow==0)
    {
        memcpy(pPixel, pSource, descriptor.getByteSize()*imageBuffer.getNbElements());
    }
    else
    {
        for(BT::uint64 rowID = 0; rowID < height; ++rowID)
        {
            memcpy(pPixel, pSource, descriptor.getByteSize() * width);
            pSource += pitchRow;
            pPixel  += descriptor.getByteSize() * width;
        }
    }
    

    // Change orientation to Y down
    FreeImage_FlipVertical(_imageData);

    BT_ASSERT(!isNull());

    /*
    #pragma omp parallel for schedule(static, 1)
    for(int64 szPixel=0; szPixel < (int64)(szWidth*szHeight); ++szPixel)
    {
        pPixel[szPixel] =
    }
    */
    BT_POST_CONDITION(isLoaded());
}

void Image::saveImage(const BT::StringOS& strFilename)
{
    BT_PRE_CONDITION(!isNull());

    //Check we have a picture
    if(_imageData == NULL)
    {
        BT_THROW_ERROR_1(u8"BasicError", u8"NULLPointerError", u8"m_pImageData");
    }

    _filename = strFilename;

    //Try to guess the file format from the file extension
    const FREE_IMAGE_FORMAT ImageFormat = FreeImage_GetFIFFromFilenameU(strFilename.c_str());
    if(ImageFormat == FIF_UNKNOWN)
    {
        BT_THROW_ERROR_1(u8"ModuleError", u8"FIUnknownFileError", BT::convertToU8(strFilename));
    }

    //Check that the plugin has sufficient writing and export capabilities ...
    const unsigned int bpp = FreeImage_GetBPP(_imageData);
    if(FreeImage_FIFSupportsWriting(ImageFormat) && FreeImage_FIFSupportsExportBPP(ImageFormat, bpp))
    {
        // ok, we can save the file
        if(FreeImage_SaveU(ImageFormat, _imageData, strFilename.c_str(), 0) == FALSE)
        {
            BT_THROW_ERROR_1(u8"ModuleError", u8"FISaveFileError", BT::convertToU8(strFilename));
        }
    }
    else
    {
        BT_THROW_ERROR_1(u8"ModuleError", u8"FISaveFileError", BT::convertToU8(strFilename));
    }
}

bool Image::compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                    size_t* nbDefectPixels, double* distance) const
{
    const BT::uint32 level = 0;
    const RT::Array3ui extents = getExtents(level);
    const bool equal = extents == reference.getExtents(level);
    if(!equal)
    {
#ifndef NDEBUG
        const RT::Array3ui refExtents = reference.getExtents(level);
        BT::String message = BT::String(u8"Comparison image: different size: ") + std::to_string(extents.x) + BT::String(u8"x") + std::to_string(extents.y)
            + BT::String(u8" != ")
            + std::to_string(refExtents.x) + BT::String(u8"x") + std::to_string(refExtents.y)
            + BT::String(u8"\nComparison: FAILURE");
        BT_PRINT_MESSAGE(message);
#endif
        return equal;
    }
    BT::ColorUC32 const* const src = getData<BT::ColorUC32>(level);
    BT::ColorUC32 const* const ref = reference.getData<BT::ColorUC32>(level);

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
    const BT::String state = (nbMaxDefectPixels >= nbDefect) ? BT::String(u8"SUCCESS\n") : BT::String(u8"FAILURE\n");
    BT::String message = BT::String(u8"Comparison image: max defect distance: ") + std::to_string(max) + BT::String(u8" < ") + std::to_string(maxDistance4D)
        + BT::String(u8"\n                  pixel count: ") + std::to_string(nbDefect) + BT::String(u8" < ") + std::to_string(nbMaxDefectPixels)
        + BT::String(u8"\nComparison: ") + state;
    BT_PRINT_MESSAGE(message);
#endif
    return nbMaxDefectPixels >= nbDefect;
}

BT::uint32 ImageKTX::getLevels() const
{
    return static_cast<BT::uint32>(_glImage->levels());
}

RT::Array3ui ImageKTX::getExtents(const BT::uint32 level) const
{
    const auto ext = _glImage->extent(level);
    return {ext.x, ext.y, 1};
}

void const* const ImageKTX::getRAWData(const BT::uint32 level) const
{
    return _glImage->data(0, 0, level);
}

}