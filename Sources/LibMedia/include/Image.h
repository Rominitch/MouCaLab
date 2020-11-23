#pragma once

#include <LibRT/include/RTImage.h>

namespace Media
{
    class Image : public RT::Image
    {
        protected:
            std::unique_ptr<gli::texture> _glImage;
            FIBITMAP*     _imageData;
            RT::Array3ui  _extents;

        public:
            Image() :
            _imageData(NULL),
            _extents(0,0,0)
            {
                BT_PRE_CONDITION(isNull());
            }

            ~Image() override
            {
                BT_PRE_CONDITION(!isLoaded());
            }

            void openImage(const BT::StringOS& strFilename) override;

            void createImage(const RT::BufferCPUBase& ImageBuffer, const BT::uint64 szWidth, const BT::uint64 szHeight) override;

            bool isNull() const override
            {
                return _filename.empty() && !isLoaded();
            }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is loaded.
            /// 
            /// \returns True if loaded, otherwise false.
            bool isLoaded() const override
            {
                return !( _imageData == NULL && _glImage.get() == nullptr );
            }

            

            void release() override;

            void saveImage(const BT::StringOS& strFilename) override;

            BT::uint32 getLevels() const override
            {
                return 1;
            }

            RT::Array3ui getExtents(const BT::uint32 level) const override
            {
                return _extents;
            }

//             template<typename DataType>
//             DataType const*const getData() const
//             {
//                 return reinterpret_cast<DataType const*const>( FreeImage_GetBits(_imageData) );
//             }

            void const*const getRAWData(const BT::uint32 level) const override
            {
                BT_PRE_CONDITION(level < getLevels());
                return FreeImage_GetBits(_imageData);
            }

//             const gli::texture& getTextureBuffer() const override
//             {
//                 return *_glImage;
//             }

            bool compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                         size_t* nbDefectPixels=nullptr, double* distance4D=nullptr) const override;
    };

    class ImageFI : public Image
    {
    protected:
        FIBITMAP*       _imageData; ///< FreeImage data
        RT::Array3ui    _extents;   ///< Extents

    public:
        ImageFI() :
            _imageData(NULL),
            _extents(0, 0, 0)
        {
            BT_PRE_CONDITION(isNull());
        }

        ~ImageFI() override
        {
            BT_PRE_CONDITION(!isLoaded());
        }

        bool isNull() const override
        {
            return _filename.empty() && !isLoaded();
        }

        //------------------------------------------------------------------------
        /// \brief  Check if resource is loaded.
        /// 
        /// \returns True if loaded, otherwise false.
        bool isLoaded() const override
        {
            return _imageData != NULL;
        }

        void release() override;

        void saveImage(const BT::StringOS& strFilename) override;

        BT::uint32 getLevels() const override
        {
            return 1;
        }

        RT::Array3ui getExtents(const BT::uint32 level) const override
        {
            return _extents;
        }

        void const* const getRAWData(const BT::uint32 level) const override
        {
            BT_PRE_CONDITION(level < getLevels());
            return FreeImage_GetBits(_imageData);
        }

        bool compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
                     size_t* nbDefectPixels = nullptr, double* distance4D = nullptr) const override;
    };

    class ImageKTX : public RT::Image
    {
    protected:
        std::unique_ptr<gli::texture> _glImage;

    public:
        ImageKTX()
        {
            BT_PRE_CONDITION(isNull());
        }

        ~ImageKTX() override
        {
            BT_PRE_CONDITION(!isLoaded());
        }

        void openImage(const BT::StringOS& strFilename) override;

        bool isNull() const override
        {
            return _filename.empty() && !isLoaded();
        }

        //------------------------------------------------------------------------
        /// \brief  Check if resource is loaded.
        /// 
        /// \returns True if loaded, otherwise false.
        bool isLoaded() const override
        {
            return _glImage.get() != nullptr;
        }

        void release() override;

        BT::uint32 getLevels() const override;

        RT::Array3ui getExtents(const BT::uint32 level) const override;

        void const* const getRAWData(const BT::uint32 level) const override;

        bool compare(const RT::Image& reference, const size_t nbMaxDefectPixels, const double maxDistance4D,
            size_t* nbDefectPixels = nullptr, double* distance4D = nullptr) const override;
    };
}