/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreString.h>

namespace Core
{
    class File;

    //----------------------------------------------------------------------------
    /// \brief This object allows to write or read flow of data.
    /// This object allows to write or read flow of data.
    /// The objective is to be very quick so we remove release overflow control.
    /// \code{ .cpp }
    ///     //How to use: Write into file.
    ///     Core::ByteBuffer buf;
    ///     buf << int(13) << 14 << 16;
    ///     Core::File file("myfile.dat");
    ///     file.open(L"w");
    ///     buf.write(file);
    ///     file.close();
    /// \endcode
    class ByteBuffer
    {
        public:
            /// Constructor
            ByteBuffer() = default;
            /// Destructor
            ~ByteBuffer() = default;

            //------------------------------------------------------------------------
            /// \brief  Return how many bytes are allocated into buffer.
            /// 
            /// \returns Allocated bytes into buffer.
            size_t size() const
            {
                return _buffer.size();
            }

            //------------------------------------------------------------------------
            /// \brief  Clean, copy buffer to byteBuffer and allow to read.
            /// 
            /// \param[in] buffer: where read bytes.
            /// \param[in] sizeInByte: how many bytes to read (WARNING: file overflow possible).
            void readBuffer(const void* buffer, const size_t sizeInByte);

            //------------------------------------------------------------------------
            /// \brief  Read bytes from file and copy on cleared buffer.
            /// 
            /// \param[in,out] file: where read bytes (you can seek before).
            /// \param[in] size: how many bytes to read (WARNING: file overflow possible).
            /// \returns Total in Byte of reading data.
            size_t read(File& file, const size_t size);

            //------------------------------------------------------------------------
            /// \brief  Write bytes to file.
            /// 
            /// \param[in,out] file: where write bytes (you can seek before).
            /// \returns Total in Byte of writing data.
            size_t write(File& file) const;
            
            //------------------------------------------------------------------------
            /// \brief Generic writer of byte (possible if sizeof() return number of bytes).
            /// 
            /// \param[in] data: data to save.
            /// \returns New buffer with element.
            template<typename DataType>
            ByteBuffer& operator<<(const DataType& data)
            {
                const MemoryByte* dataA = reinterpret_cast<const MemoryByte*>(&data);
                _buffer.insert(_buffer.end(), dataA, &dataA[sizeof(data)]);
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Write another byte buffer.
            /// 
            /// \param[in] data: data to save.
            /// \returns New buffer with element.
            template<>
            ByteBuffer& operator<<(const ByteBuffer& data)
            {
                _buffer.insert(_buffer.end(), data._buffer.data(), &data._buffer.data()[data._buffer.size()]);
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Write another byte buffer.
            /// 
            /// \param[in] data: data to save.
            /// \returns New buffer with element.
            template<>
            ByteBuffer& operator<<(const Core::String& data)
            {
                *this << data.size();
                _buffer.insert(_buffer.end(), data.data(), &data.data()[data.size()]);
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Writer of std::vector. Contained type MUST be savable using <<.
            /// 
            /// \param[in] data: data to save.
            /// \returns New buffer with element.
            template<typename DataType>
            ByteBuffer& operator<<(const std::vector<DataType>& data)
            {
                *this << data.size();
                for (const auto& element : data)
                    *this << element;
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Reader of generic data.
            /// 
            /// \param[out] data: read data.
            /// \returns New buffer with jump to next element.
            template<typename DataType>
            ByteBuffer& operator>>(DataType& data)
            {
                MOUCA_ASSERT(_readPointer + sizeof(DataType) <= _buffer.size());

                memcpy(&data, reinterpret_cast<const MemoryByte*>(&_buffer[_readPointer]), sizeof(DataType));
                _readPointer += sizeof(DataType);
                return *this;
            }           

            //------------------------------------------------------------------------
            /// \brief Reader of Core::String.
            /// 
            /// \param[out] data: read data.
            /// \returns New buffer with jump to next element.
            template<>
            ByteBuffer& operator>>(Core::String& data)
            {
                size_t sizeS = 0;
                *this >> sizeS;

                MOUCA_ASSERT(_readPointer + sizeS * sizeof(char) <= _buffer.size());
                data.resize(sizeS);
                memcpy(data.data(), reinterpret_cast<const MemoryByte*>(&_buffer[_readPointer]), sizeS);
                _readPointer += sizeS;
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Writer of std::pair. Contained type MUST be savable using <<.
            /// 
            /// \param[in] data: data to save.
            /// \returns New buffer with element.
            template<typename DataType, typename DataTypeB>
            ByteBuffer& operator<<(const std::pair<DataType, DataTypeB>& data)
            {
                *this << data.first << data.second;
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Reader of std::vector.
            /// 
            /// \param[out] data: read data.
            /// \returns New buffer with jump to next element.
            template<typename DataType>
            ByteBuffer& operator>>(std::vector<DataType>& data)
            {
                size_t size=0;
                *this >> size;
                // Allocate
                data.resize(size);
                // Fill
                for(auto& element : data)
                    *this >> element;
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief Reader of std::pair.
            /// 
            /// \param[out] data: read data.
            /// \returns New buffer with jump to next element.
            template<typename DataType, typename DataTypeB>
            ByteBuffer& operator>>(std::pair<DataType, DataTypeB>& data)
            {
                *this >> data.first >> data.second;
                return *this;
            }

            //------------------------------------------------------------------------
            /// \brief  Copy ALL current buffer to Hex string.
            /// 
            /// \returns Hex string
            String exportHex() const
            {
                std::stringstream ss;
                ss << std::hex << std::setfill('0');
                for(unsigned char i : _buffer)
                {
                    ss << std::setw(2) << static_cast<unsigned int>(i);
                }
                return ss.str();
            }

            void importHex(const String& in)
            {
                MOUCA_PRE_CONDITION((in.length() % 2) == 0);

                std::string output;
                size_t cnt = in.length() / 2;
                for(size_t i = 0; cnt > i; ++i)
                {
                    uint32_t s = 0;
                    std::stringstream ss;
                    ss << std::hex << in.substr(i * 2, 2);
                    ss >> s;

                    *this << static_cast<unsigned char>(s);
                }
            }

            //------------------------------------------------------------------------
            /// \brief  Access to memory buffer. WARNING: NEVER DELETE AND USE WITH PRECAUTION !
            /// 
            /// \returns Internal buffer data.
            const void* getData() const
            {
                return _buffer.data();
            }

        private:
            using MemoryByte = unsigned char;
            using MemoryArray = std::vector<MemoryByte>;

            MemoryArray _buffer;                ///< Buffer memory.
            std::size_t _readPointer = 0;       ///< Pointer where we are into buffer.
    };

}

