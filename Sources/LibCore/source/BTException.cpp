/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibCore/include/BTException.h>

#define DUMP_MESSAGE

namespace Core
{

Exception::Exception(const ErrorData& pFirstError) noexcept
{
#if !defined NDEBUG && defined DUMP_MESSAGE
    // Simple dump of exception data
    std::cerr << u8"----- Exception --------------"  << std::endl
              << "Location:\t" << pFirstError.getFile() << u8"(" << pFirstError.getLine() << u8") " << std::endl
              << "Code:\t\t"   <<  pFirstError.getLibraryLabel() << u8" " << pFirstError.getErrorLabel() << std::endl;
    for( const auto& parameters : pFirstError.getParameters())
    {
        std::cerr << u8"\t" << parameters << std::endl;
    }
    std::cerr << std::endl;
#endif
    
    //Add first error
    _errorsHierarchy.emplace_front(std::move(pFirstError));
}

Exception::~Exception() noexcept
{
    _errorsHierarchy.clear();
}

void Exception::addCatchedError(const ErrorData& pLastError) noexcept
{
    _errorsHierarchy.push_front(pLastError);
}

const ErrorData& Exception::read(const size_t szId) const noexcept
{
    BT_ASSERT(szId < _errorsHierarchy.size());

    return _errorsHierarchy[szId];
}

size_t Exception::getNbErrors() const noexcept
{
    return _errorsHierarchy.size();
}

}