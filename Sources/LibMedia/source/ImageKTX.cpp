#include "Dependencies.h"

#include "LibMedia/include/ImageKTX.h"

#include "LibMedia/include/ImageLoader.h"

namespace Media
{

void ImageKTX::initialize(const Core::Path& path)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!path.empty());

    ktxResult result = KTX_SUCCESS;
#if defined(__ANDROID__)
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, path.c_str(), AASSET_MODE_STREAMING);
    if (!asset) {
        vks::tools::exitFatal("Could not load texture from " + filename + "\n\nThe file may be part of the additional asset pack.\n\nRun \"download_assets.py\" in the repository root to download the latest version.", -1);
    }
    size_t size = AAsset_getLength(asset);
    assert(size > 0);
    ktx_uint8_t* textureData = new ktx_uint8_t[size];
    AAsset_read(asset, textureData, size);
    AAsset_close(asset);
    result = ktxTexture_CreateFromMemory(textureData, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &_images);
    delete[] textureData;
#else
    result = ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &_images);
#endif
    if(result != KTX_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("LibMedia", "KTXReadError") << path.string());
    }

    MouCa::preCondition(!isNull());
}

void ImageKTX::createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height)
{
    throw Core::Exception(Core::ErrorData("BasicError", "ImageNoneImplemented"));
}

void ImageKTX::release()
{
    MouCa::preCondition(!isNull());

    ktxTexture_Destroy(_images);
    _images = nullptr;

    MouCa::postCondition(isNull());
}

uint32_t ImageKTX::getLayers() const
{
    MouCa::preCondition(!isNull());

    return _images->numLayers;
}

uint32_t ImageKTX::getLevels() const
{
    MouCa::preCondition(!isNull());
    return _images->numLevels;
}

RT::Array3ui ImageKTX::getExtents(const uint32_t level) const
{
    MouCa::preCondition(!isNull());
    return
    {
        std::max(1u, _images->baseWidth  >> level),
        std::max(1u, _images->baseHeight >> level),
        std::max(1u, _images->baseDepth  >> level)
    };
}

const ImageKTX::HandlerMemory ImageKTX::getRAWData(const uint32_t layer, const uint32_t level) const
{
    MouCa::preCondition(!isNull());

    ktx_size_t offset;
    auto result = ktxTexture_GetImageOffset(_images, level, layer, 0, &offset);
    MouCa::assertion(result == KTX_SUCCESS);
    return ktxTexture_GetData(_images) + offset;
}

size_t ImageKTX::getMemoryOffset(const uint32_t layer, const uint32_t level) const
{
    MouCa::preCondition(!isNull());

    size_t offset = 0;
    auto result = ktxTexture_GetImageOffset(_images, level, layer, 0, &offset);
    MouCa::assertion(result == KTX_SUCCESS);
    return offset;
}

size_t ImageKTX::getMemorySize() const
{
    MouCa::preCondition(!isNull());
    return _images->dataSize;
}

void ImageKTX::saveImage(const Core::Path& filename)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!filename.empty());

    if(ktxTexture_WriteToNamedFile(_images, filename.string().c_str()))
    {
        throw Core::Exception(Core::ErrorData("BasicError", "ImageKTXSave") << filename.string());
    }
}

void ImageKTX::export2D(const Core::Path& filename)
{
    ImageLoader::export2D(*this, filename);
}

bool ImageKTX::compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                       size_t* nbDefectPixels, double* distance4D) const
{
    throw Core::Exception(Core::ErrorData("BasicError", "ImageNoneImplemented"));
}

ImageKTX::Target ImageKTX::getTarget() const
{
    MouCa::preCondition(!isNull());
    
    Image::Target target = Target::Type1D;

    bool isArray = _images->numLayers > 1;
    if(_images->baseHeight > 1)
    {
        if (_images->baseDepth > 1)
        {
            target = Target::Type3D;
        }
        else
        {
            target = isArray ? Target::Type2DArray : Target::Type2D;
        }
    }
    else
    {
        target = isArray ? Target::Type1DArray : Target::Type1D;
    }
    return target;
}

}