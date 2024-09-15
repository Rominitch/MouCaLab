/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibSteam/include/Platform.h"

namespace Steam
{

Platform::Platform():
_initialize(false)
{
    MouCa::postCondition(isNull());                /// DEV Issue: missing call release() ?
}

Platform::~Platform()
{
    MouCa::postCondition(isNull());                /// DEV Issue: missing call release() ?
}

bool Platform::isNull() const
{
    return !_initialize;
}

void Platform::initialize()
{
    MouCa::preCondition(isNull());

    if(SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
        throw Core::Exception(Core::ErrorData("Steam", "APINeedRestartError"));
    
    if(!Steamworks_InitCEGLibrary())
        throw Core::Exception(Core::ErrorData("Steam", "CEGInitializationError"));

    if(!SteamAPI_Init())
        throw Core::Exception(Core::ErrorData("Steam", "APIInitializationError"));

    // DRM self check
    Steamworks_SelfCheck();

    if(!SteamInput()->Init())
        throw Core::Exception(Core::ErrorData("Steam", "InputInitializationError"));

    _initialize = true;

    MouCa::postCondition(!isNull());
}

void Platform::release()
{
    MouCa::preCondition(!isNull());

    // Shutdown the SteamAPI
    SteamAPI_Shutdown();

    // Shutdown Steam CEG
    Steamworks_TermCEGLibrary();

    _initialize = false;

    MouCa::postCondition(isNull());
}

}