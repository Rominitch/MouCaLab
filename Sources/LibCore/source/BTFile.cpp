/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibCore/include/BTByteBuffer.h>
#include <LibCore/include/BTDefine.h>
#include <LibCore/include/BTException.h>
#include <LibCore/include/BTFile.h>

namespace Core
{

void File::open(const StringOS& mode)
{
    BT_PRE_CONDITION(!_filename.empty());

    close();

    const errno_t eError = _wfopen_s(&_file, _filename.c_str(), mode.c_str());
    if(eError!=0)
    {
        BT_THROW_ERROR_1(u8"BasicError", u8"InvalidPathError", convertToU8(_filename));
    }
    else
    {
        m_szStartFile = getFilePosition();
    }
}

void File::close()
{
    if(_file!=nullptr)
    {
        fclose(_file);
        _file=nullptr;
    }
}

void File::release()
{
    close();
}

bool File::isExist() const
{
    return !_filename.empty() && std::filesystem::exists(std::filesystem::path(_filename));
}

bool File::isOpened() const
{
    return isExist() && _file!=nullptr;
}

bool File::isExist(const Path& filePath)
{
    BT_PRE_CONDITION(!filePath.empty());
    return std::filesystem::exists(filePath);
}

size_t File::read(const size_t iPositionInFile, void* pBuffer, const size_t szSizeToReadInByte) const
{
    BT_PRE_CONDITION(_file != nullptr);
    BT_PRE_CONDITION(pBuffer != nullptr && szSizeToReadInByte > 0);

    //Set reading head to good position
    setFilePosition(iPositionInFile);
    return fread(pBuffer, sizeof(char), szSizeToReadInByte, _file);
}

size_t File::write(const size_t iPositionInFile, const void* pBuffer, const size_t szSizeToWriteInByte)
{
    BT_PRE_CONDITION(_file != nullptr);
    BT_PRE_CONDITION(pBuffer != nullptr && szSizeToWriteInByte > 0);

    ///Set reading head to good position
    setFilePosition(iPositionInFile);
    return fwrite(pBuffer, sizeof(char), szSizeToWriteInByte, _file);
}

size_t File::read(void* pBuffer, const size_t szSizeToReadInByte) const
{
    BT_PRE_CONDITION(_file != nullptr);
    BT_PRE_CONDITION(pBuffer != nullptr && szSizeToReadInByte > 0);

    return fread(pBuffer, sizeof(char), szSizeToReadInByte, _file);
}

size_t File::write(const void* pBuffer, const size_t szSizeToWriteInByte)
{
    BT_PRE_CONDITION(_file != nullptr);
    BT_PRE_CONDITION(pBuffer != nullptr && szSizeToWriteInByte > 0);

    return fwrite(pBuffer, sizeof(char), szSizeToWriteInByte, _file);
}

size_t File::read(Core::ByteBuffer& buffer)
{
    //Read Buffer size
    uint64_t memorySize=0;
    size_t mem = read(&memorySize, sizeof(uint64_t));

    // Check we can read data into file
    if(memorySize <= (getSizeOfFile() - getFilePosition()))
    {
        mem += buffer.read(*this, memorySize);
    }
    else
    {
        BT_THROW_ERROR_1(u8"BasicError", u8"InvalidFileRead", convertToU8(_filename));
    }
    return mem;
}

size_t File::write(const Core::ByteBuffer& buffer)
{
    const uint64_t memorySize=buffer.size();
    size_t mem = write(&memorySize, sizeof(uint64_t));
    mem += buffer.write(*this);

    return mem;
}

size_t File::write(const std::stringstream& stream)
{
    return write(stream.str().c_str(), stream.str().size()*sizeof(char));
}

StringUTF8 File::extractUTF8() const
{
    return extractString();
}

std::string File::extractString() const
{
    const size_t fileSize = getSizeOfFile();
    if(fileSize == 0)
        return std::string();

    //Create buffer
    std::vector<char> txtline;
    txtline.resize(fileSize);

    //Read all file
    read(0, txtline.data(), fileSize);

    //Build converter and return string
    return std::string(txtline.begin(), txtline.end());
}

size_t File::getSizeOfFile() const
{
    BT_PRE_CONDITION(_file != nullptr);

    return std::filesystem::file_size(_filename);
}

void File::setFilePosition(const size_t positionInFile) const
{
    BT_PRE_CONDITION(_file != nullptr);
    const fpos_t pPosition = static_cast<const fpos_t>(positionInFile + m_szStartFile);

    if(fsetpos(_file, &pPosition) != 0)
    {
        BT_THROW_ERROR(u8"BasicError", u8"InvalidReadingPositionError");
    }
}

size_t File::getFilePosition() const
{
    BT_PRE_CONDITION(_file != nullptr);
    fpos_t pPosition = 0;
    if(fgetpos(_file, &pPosition) != 0)
    {
        BT_THROW_ERROR(u8"BasicError", u8"InvalidReadingPositionError");
    }
    return (size_t)pPosition;
}

Path File::getFilePath() const
{
    return getFilename();
}

void File::writeText(const char* format, ...)
{
    BT_PRE_CONDITION(_file != nullptr);

    va_list pl;
    va_start(pl, format);

    vfprintf_s(_file, format, pl);

    va_end(pl);

    BT_POST_CONDITION(_file != nullptr);
}


}