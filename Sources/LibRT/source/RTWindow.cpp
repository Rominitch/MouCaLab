/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTWindow.h"

namespace RT
{

Window::WindowInstance Window::getInstance() const
{
#ifdef MOUCA_OS_WINDOWS
     return GetModuleHandle(nullptr);
#else
     return nullptr;
#endif
}

}