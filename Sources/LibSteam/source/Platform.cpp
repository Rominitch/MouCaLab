/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibSteam/include/Platform.h"

namespace Steam
{

Platform::Platform():
_initialize(false)
{
    BT_POST_CONDITION(isNull());                /// DEV Issue: missing call release() ?
}

Platform::~Platform()
{
    BT_POST_CONDITION(isNull());                /// DEV Issue: missing call release() ?
}

bool Platform::isNull() const
{
    return !_initialize;
}

void Platform::initialize()
{
    BT_PRE_CONDITION(isNull());

    if(SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
        BT_THROW_ERROR(u8"Steam", u8"APINeedRestartError");
    
    if(!Steamworks_InitCEGLibrary())
        BT_THROW_ERROR(u8"Steam", u8"CEGInitializationError");

    if(!SteamAPI_Init())
        BT_THROW_ERROR(u8"Steam", u8"APIInitializationError");

    // DRM self check
    Steamworks_SelfCheck();

    if(!SteamInput()->Init())
        BT_THROW_ERROR(u8"Steam", u8"InputInitializationError");

    _initialize = true;

    BT_POST_CONDITION(!isNull());
}

void Platform::release()
{
    BT_PRE_CONDITION(!isNull());

    // Shutdown the SteamAPI
    SteamAPI_Shutdown();

    // Shutdown Steam CEG
    Steamworks_TermCEGLibrary();

    _initialize = false;

    BT_POST_CONDITION(isNull());
}

}