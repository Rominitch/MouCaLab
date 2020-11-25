/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibCore/include/CoreIdentifier.h"

namespace Core
{

std::atomic_uint_fast64_t Identifier::_latestGID = 0;

}