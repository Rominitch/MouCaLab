#pragma once

#include <LibRT/include/RTImage.h>

namespace Media
{
    class ImageFI : public RT::Image
    {
        protected:
            FIBITMAP*       _imageData; ///< FreeImage data

        public:
            ImageFI() :
            _imageData(nullptr)
            {
                MOUCA_PRE_CONDITION(isNull());
            }

            ~ImageFI() override
            {
                MOUCA_PRE_CONDITION(isNull());
            }

            void initialize(const Core::Path& path);

            void createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height) override;

            bool isNull() const override
            {
                return _imageData == nullptr;
            }

            Target getTarget() const override { return Target::Type2D; }

            void release() override;

            void saveImage(const Core::Path& filename) override;

            uint32_t getLayers() const override   { return 1; }
            
            uint32_t getLevels() const override   { return 1; }

            RT::Array3ui getExtents(const uint32_t level) const override;

            size_t getMemoryOffset(const uint32_t layer, const uint32_t level) const override;

            size_t getMemorySize() const override;

            void const* const getRAWData(const uint32_t layer, const uint32_t level) const override
            {
                MOUCA_PRE_CONDITION(level < getLevels());
                BT_UNUSED(layer);
                BT_UNUSED(level);
                return FreeImage_GetBits(_imageData);
            }

            bool compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                         size_t* nbDefectPixels = nullptr, double* distance4D = nullptr) const override;
    };
}