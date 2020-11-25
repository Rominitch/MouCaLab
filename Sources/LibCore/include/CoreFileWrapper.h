/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreString.h>

namespace Core
{
    class ByteBuffer;

    //----------------------------------------------------------------------------
    /// \brief Abstract reader/writer file. To be override by nice API/3rd part.
    ///
    /// \see   File
    class FileWrapperBase
    {
        public:
            /// Destructor
            virtual ~FileWrapperBase(void) = default;

        //------------------------------------------------------------------------------------------
        //									Open/Close methods
        //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Open file calling OS.
            /// 
            /// \param[in] strMode: how to open file.
            /// \note You MUST call File() or initialize() with right filename BEFORE.
            /// \note Can be call many time.
            virtual void open(const StringOS& strMode=L"r+") = 0;

            //------------------------------------------------------------------------
            /// \brief  Close file.
            /// 
            /// \note You MUST call open() BEFORE.
            virtual void close() = 0;

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
            virtual size_t read(const size_t iPositionInFile, void* pBuffer, const size_t szSizeToReadInByte) const = 0;

            //------------------------------------------------------------------------
            /// \brief  Write data into buffer.
            /// 
            /// \param[in] iPositionInFile: where write into file.
            /// \param[in] pBuffer: buffer of byte.
            /// \param[in] szSizeToWriteInByte: size of buffer.
            /// \returns Total of successful written byte.
            virtual size_t write(const size_t iPositionInFile, const void* pBuffer, const size_t szSizeToWriteInByte) = 0;

            //------------------------------------------------------------------------
            /// \brief  Read data into buffer from latest position read/write.
            /// 
            /// \param[in,out] pBuffer: buffer of byte.
            /// \param[in] szSizeToReadInByte: size of buffer.
            /// \returns Total of successful read byte.
            virtual size_t read(void* pBuffer, const size_t szSizeToReadInByte) const = 0;

            //------------------------------------------------------------------------
            /// \brief  Write data into buffer from latest position read/write.
            /// 
            /// \param[in] pBuffer: buffer of byte.
            /// \param[in] szSizeToWriteInByte: size of buffer.
            /// \returns Total of successful written byte.
            virtual size_t write(const void* pBuffer, const size_t szSizeToWriteInByte) = 0;

            //------------------------------------------------------------------------
            /// \brief  Read data into ByteBuffer from latest position read/write.
            /// 
            /// \param[in,out] buffer: buffer structure to edit.
            /// \returns Total of successful read byte.
            virtual size_t read(Core::ByteBuffer& buffer) = 0;

            //------------------------------------------------------------------------
            /// \brief  Write data into ByteBuffer from latest position read/write.
            /// 
            /// \param[in] buffer: buffer structure with all data.
            /// \returns Total of successful written byte.
            virtual size_t write(const Core::ByteBuffer& buffer) = 0;

            //------------------------------------------------------------------------
            /// \brief  Write data from stream to latest position read/write.
            /// 
            /// \param[in] stream: string stream with all data.
            /// \returns Total of successful written byte.
            virtual size_t write(const std::stringstream& stream) = 0;

            //------------------------------------------------------------------------
            /// \brief  Write data from text to latest position read/write.
            /// 
            /// \param[in] format: format of text.
            virtual void writeText(const char* format, ...) = 0;

        //------------------------------------------------------------------------------------------
        //									Getter/Setter methods
        //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Get number of byte into file.
            /// 
            /// \returns Number of byte into file.
            virtual size_t getSizeOfFile() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Change writer/reader position into file.
            /// 
            /// \param[in] iPositionInFile: new position into file.
            /// \throw Exception if impossible to reach position.
            virtual void setFilePosition(const size_t iPositionInFile) const = 0;

            //------------------------------------------------------------------------
            /// \brief  Get writer/reader position into file.
            /// 
            /// \returns Writer/reader position into file.
            virtual size_t getFilePosition() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Get Filepath.
            /// 
            /// \returns Filepath.
            virtual Path getFilePath() const = 0;

     // NOW -> Byte buffer 
     //       //------------------------------------------------------------------------
     //       /// \brief  Operator for write 
     //       /// 
     //       /// \param[in,out]  
     //       template<typename DataType>
     //       void operator>>(DataType& data)
     //       {
     //           read(getFilePosition(), data, sizeof(DataType));
     //       }
     //
     //       template<typename DataType>
     //       void operator<<(const DataType& data)
     //       {
     //           write(getFilePosition(), data, sizeof(DataType));
     //       }

        protected:
            /// Constructor (from inherited class)
            FileWrapperBase(void) = default;
    };
}