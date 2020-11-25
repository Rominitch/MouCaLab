/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreFileWrapper.h>
#include <LibCore/include/CoreResource.h>

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Basic reader/writer file based on standard C++ tools.
    ///
    /// \see   FileWrapperBase, Resource
    class File : public FileWrapperBase, public Resource
    {
        protected:
            FILE*		_file;            ///< Standard c++ FILE handle
            fpos_t		m_szStartFile;      ///< Reader/Writer position.

        public:
            //------------------------------------------------------------------------
            /// \brief  Constructor
            /// 
            /// \param[in] strFilePath: where read/write.
            /// \param[in] eMode: kind of resource is this file.
            File(const Path& strFilePath = Path(), const Mode eMode = Mode::NoRefresh):
            Resource(strFilePath, eMode), _file(nullptr), m_szStartFile(0)
            {}
        
            /// Destructor
            ~File() override
            {
                //Close automatically
                close();
            }

        //------------------------------------------------------------------------------------------
        //									Open/Close methods
        //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Open file calling OS.
            /// 
            /// \param[in] strMode: how to open file.
            /// \note You MUST call File() or initialize() with right filename BEFORE.
            /// \note Can be call many time.
            void open(const StringOS& strMode=L"r+b") override;

            //------------------------------------------------------------------------
            /// \brief  Close file.
            /// 
            /// \note You MUST call open() BEFORE.
            void close() override;

            void release() override;

            bool isNull() const override
            {
                return !isLoaded()
                    && _filename.empty();
            }

            bool isLoaded() const override
            {
                return _file != nullptr;
            }

        //------------------------------------------------------------------------------------------
        //									Read/Write methods
        //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Read data into buffer.
            /// 
            /// \param[in] iPositionInFile: where read into file.
            /// \param[out] pBuffer: buffer of byte.
            /// \param[in] szSizeToReadInByte: size of buffer.
            /// \returns Total of successful read byte.
            size_t read(const size_t iPositionInFile, void* pBuffer, const size_t szSizeToReadInByte) const override;

            //------------------------------------------------------------------------
            /// \brief  Write data into void buffer.
            /// 
            /// \param[in] iPositionInFile: where write into file.
            /// \param[in] pBuffer: buffer of byte.
            /// \param[in] szSizeToWriteInByte: size of buffer.
            /// \returns Total of successful written byte.
            size_t write(const size_t iPositionInFile, const void* pBuffer, const size_t szSizeToWriteInByte) override;

            //------------------------------------------------------------------------
            /// \brief  Read data into void buffer from latest position read/write.
            /// 
            /// \param[in,out] pBuffer: buffer of byte.
            /// \param[in] szSizeToReadInByte: size of buffer.
            /// \returns Total of successful read byte.
            size_t read(void* pBuffer, const size_t szSizeToReadInByte) const override;

            //------------------------------------------------------------------------
            /// \brief  Write data into void buffer from latest position read/write.
            /// 
            /// \param[in] pBuffer: buffer of byte.
            /// \param[in] szSizeToWriteInByte: size of buffer.
            /// \returns Total of successful written byte.
            size_t write(const void* pBuffer, const size_t szSizeToWriteInByte) override;

            //------------------------------------------------------------------------
            /// \brief  Read data into ByteBuffer from latest position read/write.
            /// 
            /// \param[in,out] buffer: buffer structure to edit.
            /// \returns Total of successful read byte.
            size_t read(Core::ByteBuffer& buffer) override;

            //------------------------------------------------------------------------
            /// \brief  Write data from ByteBuffer to latest position read/write.
            /// 
            /// \param[in] buffer: buffer structure with all data.
            /// \returns Total of successful written byte.
            size_t write(const Core::ByteBuffer& buffer) override;

            //------------------------------------------------------------------------
            /// \brief  Write data from stream to latest position read/write.
            /// 
            /// \param[in] stream: string stream with all data.
            /// \returns Total of successful written byte.
            size_t write(const std::stringstream& stream) override;

            //------------------------------------------------------------------------
            /// \brief  Write data from text to latest position read/write.
            /// 
            /// \param[in] format: format of text.
            void writeText(const char* format, ...) override;

        //------------------------------------------------------------------------------------------
        //									Getter/Setter methods
        //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Get number of byte into file.
            /// 
            /// \returns Number of byte into file.
            size_t getSizeOfFile() const override;

            //------------------------------------------------------------------------
            /// \brief  Change writer/reader position into file.
            /// 
            /// \param[in] iPositionInFile: new position into file.
            /// \throw Exception if impossible to reach position.
            void setFilePosition(const size_t iPositionInFile) const override;

            //------------------------------------------------------------------------
            /// \brief  Get writer/reader position into file.
            /// 
            /// \returns Writer/reader position into file.
            size_t getFilePosition() const override;

            //------------------------------------------------------------------------
            /// \brief  Get Filepath.
            /// 
            /// \returns Filepath.
            Path getFilePath() const override;

            //------------------------------------------------------------------------
            /// \brief  Check if current file exists on disk.
            /// 
            /// \returns True if file exists, false otherwise.
            bool isExist() const;

            //------------------------------------------------------------------------
            /// \brief  Check if current file exists on disk and is open.
            /// 
            /// \returns True if file exists and open, false otherwise.
            bool isOpened() const;

            //------------------------------------------------------------------------
            /// \brief  Extract all whole file into string UTF-8 buffer.
            /// 
            /// \returns UTF-8 buffer with all file data.
            StringUTF8 extractUTF8() const;

            //------------------------------------------------------------------------
            /// \brief  Extract all whole file into string buffer.
            /// 
            /// \returns Buffer with all file data.
            std::string extractString() const;

            //------------------------------------------------------------------------
            /// \brief  Check if current file exists on disk.
            /// 
            /// \returns True if file exists, false otherwise.
            static bool isExist(const Path& strFilePath);
    };

    using FileSPtr = std::shared_ptr<File>;
    using FileWPtr = std::weak_ptr<File>;
}