#include "Dependencies.h"

#include <LibCore/include/CoreDefine.h>

namespace MouCa
{

void assertHeader(const bool condition, const Core::StringView& header, const std::source_location& location)
{
#ifdef MOUCA_ACTIVE_ASSERT
    if(!condition)
    {
        auto message = std::format("{} ({}): [ERROR] {} - Bad condition into {}() \n", location.file_name(), location.line(), header, location.function_name());

        OutputDebugString(Core::convertToOS(message).c_str());
        assert(condition);
    }
#endif
}

void preCondition(const bool condition, const std::source_location& location)
{
#ifdef MOUCA_ACTIVE_ASSERT
    assertHeader(condition, "Pre-condition assert", location);
#endif
}

void postCondition(const bool condition, const std::source_location& location)
{
#ifdef MOUCA_ACTIVE_ASSERT
    assertHeader(condition, "Post-condition assert", location);
#endif
}

void assertion(const bool condition, const std::source_location& location)
{
#ifdef MOUCA_ACTIVE_ASSERT
    assertHeader(condition, "Assert", location);
#endif
}

}