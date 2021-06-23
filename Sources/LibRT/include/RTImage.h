/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreResource.h>

namespace RT
{
    class BufferCPUBase;
    using BufferCPUBaseWPtr = std::weak_ptr<BufferCPUBase>;

    //----------------------------------------------------------------------------
    /// \brief Abstract image system to read/write data.
    class Image
    {
        MOUCA_NOCOPY_NOMOVE(Image);

        public:
            using HandlerMemory = void*;

            enum class Target
            {
                TypeUnknown,
                Type1D,
                Type1DArray,
                Type2D,
                Type2DArray,
                Type3D
            };

            /// Destructor
            virtual ~Image() = default;

            //------------------------------------------------------------------------
            /// \brief Remove all internal data
            virtual void release() = 0;

            //------------------------------------------------------------------------
            /// \brief Create current image with new data. 
            /// \param[in] imageBuffer: Buffer.
            /// \param[in] width:  buffer size in width.
            /// \param[in] height: buffer size in height.
            /// \note Need to call release to make initialize again.
            virtual void createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height) = 0;

            //------------------------------------------------------------------------
            /// \brief Save image data in current format on disk if possible (and if implemented).
            /// \param[in] filename: where save image on disk.
            /// \throw  If feature is not implemented or error.
            virtual void saveImage(const Core::Path& filename) = 0;

            //------------------------------------------------------------------------
            /// \brief Save image data in valid format on disk if possible (and if implemented).
            /// \param[in] filename: where save image on disk.
            /// \throw  If feature is not implemented or error.
            virtual void export2D(const Core::Path& filename) = 0;

            //------------------------------------------------------------------------
            /// \brief  Check if image exists.
            /// 
            /// \returns True if no data, otherwise false.
            virtual bool isNull() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Compare current picture about pixel method: compute distance 4D (RGBA) between all pixels and reply false if more defect pixel than expected.
            /// 
            /// \returns False if more defect pixel than expected, true otherwise.
            virtual bool compare(const Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                                 size_t* nbDefectPixels=nullptr, double* distance4D=nullptr) const = 0;
            
            //------------------------------------------------------------------------
            /// \brief Get minimal valid target of texture.
            /// \returns Returns Target enum.
            virtual Target getTarget() const = 0;

            //------------------------------------------------------------------------
            /// \brief Get accessible mipmap level
            /// \returns Number of levels accessible.
            virtual uint32_t getLevels() const = 0;

            //------------------------------------------------------------------------
            /// \brief Get accessible layers.
            /// \returns Number of layers accessible.
            virtual uint32_t getLayers() const = 0;

            //------------------------------------------------------------------------
            /// \brief Get extents for each mipmap level.
            /// \param[in] level: mipmap level to get size.
            /// \returns Specific extents for one mipmap level.
            virtual RT::Array3ui getExtents(const uint32_t level) const = 0;

            //------------------------------------------------------------------------
            /// \brief Get pointer to first data of texture based on layer/level.
            /// \param[in] layer: id of layer.
            /// \param[in] level: mipmap level to get size.
            /// \returns Pointer to first data.
            virtual const HandlerMemory getRAWData(const uint32_t layer, const uint32_t level) const = 0;

            //------------------------------------------------------------------------
            /// \brief Get casted pointer to first data of texture based on layer/level.
            /// \param[in] layer: id of layer.
            /// \param[in] level: mipmap level to get size.
            /// \returns Casted pointer to first data.
            template<typename DataType>
            DataType const * const getData(const uint32_t layer, const uint32_t level) const
            {
                return reinterpret_cast<DataType const*const>(getRAWData(layer, level));
            }

            //------------------------------------------------------------------------
            /// \brief Get memory offset based on layer/level.
            /// \param[in] layer: id of layer.
            /// \param[in] level: mipmap level to get size.
            /// \returns Memory offset of first data.
            virtual size_t getMemoryOffset(const uint32_t layer, const uint32_t level) const = 0;
            
            //------------------------------------------------------------------------
            /// \brief Get full memory size of texture.
            /// \returns Full memory size of texture.
            virtual size_t getMemorySize() const = 0;

        protected:
            /// Constructor
            Image() = default;
    };

    using ImageSPtr = std::shared_ptr<Image>;
    using ImageWPtr = std::weak_ptr<Image>;

    //----------------------------------------------------------------------------
    /// \brief Image based on linkedCPU buffer. This class not manage memory of data BUT DEV must guaranty still valid !!!
    ///
    class ImageLinkedCPU : public Image
    {
        public:
            /// Constructor
            ImageLinkedCPU() = default;
            /// Destructor
            ~ImageLinkedCPU() override = default;

            void initialize(const Target target, const RT::BufferCPUBaseWPtr& imageBuffer, const RT::Array3ui& extents);

            void release() override;

            //------------------------------------------------------------------------
            /// \brief Create current image with new data. Need to call release to make initialize again.
            /// \param[in] imageBuffer: Buffer.
            /// \param[in] width:  buffer size in width.
            /// \param[in] height: buffer size in height.
            void createFill(const RT::BufferCPUBase& imageBuffer, const uint32_t width, const uint32_t height) override;

            //------------------------------------------------------------------------
            /// \brief Not implemented
            /// \throw  Throw error.
            void saveImage(const Core::Path& ) override;

            void export2D(const Core::Path& ) override;

            //------------------------------------------------------------------------
            /// \brief Get target of texture.
            /// \returns Returns Target enum.
            Target getTarget() const override { return _target; }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is not loaded and have no link to file data.
            /// 
            /// \returns True if not loaded and no link, otherwise false.
            bool isNull() const override;

            //------------------------------------------------------------------------
            /// \brief Not implemented
            /// \throw  Throw error.
            bool compare(const Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                         size_t* nbDefectPixels = nullptr, double* distance4D = nullptr) const override;

            uint32_t getLevels() const override { return 1; }

            uint32_t getLayers() const override { return 1; }

            RT::Array3ui getExtents(const uint32_t level) const override
            {
                MOUCA_PRE_CONDITION(level < getLevels());
                MOUCA_UNUSED(level);
                return _extents;
            }

            const HandlerMemory getRAWData(const uint32_t layer, const uint32_t level) const override;

            size_t getMemoryOffset(const uint32_t layer, const uint32_t level) const override;

            size_t getMemorySize() const override;

        private:
            BufferCPUBaseWPtr _linkedBuffer;    ///< [LINK] Link to buffer CPU
            RT::Array3ui      _extents;         ///< 3D sizes.
            Target            _target;          ///< Target of texture.
    };

    //----------------------------------------------------------------------------
    /// \brief Image resource from disk.
    /// \note See Lib Media to check all formats.
    /// \see Core::Resource, Media::ImageFI, Media::ImageKTX
    class ImageImport : public Core::Resource
    {
        MOUCA_NOCOPY_NOMOVE(ImageImport);

        public:
            ImageImport() = default;
            ~ImageImport() override = default;

            void release() override
            {
                MOUCA_PRE_CONDITION(!isNull());
                if (_image != nullptr)
                {
                    MOUCA_PRE_CONDITION(_image.use_count() == 1); // DEV Issue: Not latest instance ?
                    _image->release();
                    _image.reset();
                }
                _filename.clear();

                MOUCA_POST_CONDITION(isNull());
            }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is not loaded and have no link to file data.
            /// 
            /// \returns True if not loaded and no link, otherwise false.
            bool isNull() const override
            {
                return (_image == nullptr || _image->isNull()) && _filename.empty();
            }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is loaded.
            /// 
            /// \returns True if loaded, otherwise false.
            bool isLoaded() const override
            {
                return !_filename.empty() && !_image->isNull();
            }

            void setImage(ImageSPtr image)  { _image = image; }
            ImageWPtr getImage() const      { return _image; }

        protected:
            ImageSPtr _image;
    };

    using ImageImportSPtr = std::shared_ptr<ImageImport>;
    using ImageImportWPtr = std::weak_ptr<ImageImport>;
}