/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibCore/include/BTFile.h"
#include "LibCore/include/BTByteBuffer.h"

namespace Core
{

void ByteBuffer::readBuffer(const void* buffer, const size_t sizeInByte)
{
    _buffer.clear();
    _buffer.resize(sizeInByte);
    memcpy_s(_buffer.data(), sizeInByte, buffer, sizeInByte);
    _readPointer = 0;
}

size_t ByteBuffer::read(File& file, const size_t size)
{
    BT_ASSERT(file.isExist());
    BT_ASSERT(file.getFilePosition() + size <= file.getSizeOfFile());

    _buffer.clear();
    _buffer.resize(size);
    size_t mem = file.read(_buffer.data(), size);
    _readPointer = 0;

    return mem;
}

size_t ByteBuffer::write(File& file) const
{
    BT_ASSERT(file.isExist());
    BT_ASSERT(!_buffer.empty());
    return file.write(_buffer.data(), _buffer.size());
}

}