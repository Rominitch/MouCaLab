/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibCore/include/BTString.h"
#include "LibCore/include/BTStandardOS.h"

namespace Core
{

StringOS getEnvironmentVariable(const char* nameVariable)
{
#ifdef MOUCA_OS_WINDOWS
    char* value;
    size_t len;
    _dupenv_s(&value, &len, nameVariable);
    return Core::convertToOS(Core::String(value));
#else
    return Core::StringOS(getenv(nameVariable));
#endif
}


}