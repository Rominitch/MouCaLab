#pragma once

#include <LibRT/include/RTImage.h>

namespace Media
{
    class ImageKTX : public RT::Image
    {
        public:
            ImageKTX()
            {
                BT_PRE_CONDITION(isNull());
            }

            ~ImageKTX() override
            {
                BT_PRE_CONDITION(isNull());
            }

            void initialize(const Core::Path& path);

            void createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height) override;

            void saveImage(const Core::Path& filename) override;

            Target getTarget() const override;

            bool isNull() const override
            {
                return _glImage == nullptr;
            }

            void release() override;

            uint32_t getLayers() const override;

            uint32_t getLevels() const override;

            RT::Array3ui getExtents(const uint32_t level) const override;

            void const* const getRAWData(const uint32_t layer, const uint32_t level) const override;

            size_t getMemoryOffset(const uint32_t layer, const uint32_t level) const override;

            size_t getMemorySize() const override;

            bool compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                         size_t* nbDefectPixels = nullptr, double* distance4D = nullptr) const override;

        protected:
            std::unique_ptr<gli::texture> _glImage;
    };
}