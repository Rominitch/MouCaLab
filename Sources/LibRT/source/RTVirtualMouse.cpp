/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTVirtualMouse.h"

namespace RT
{

void VirtualMouse::setMode( const Mode mode )
{
    _mode = mode;

    // Call all observers
    _onChangeMode.emit(*this);
}

}