/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibCore/include/CoreException.h"
#include "LibCore/include/CorePluginEntry.h"
#include "LibCore/include/CorePluginManager.h"

#ifdef MOUCA_OS_WINDOWS
#   include <windows.h>
#   define DYNLIB_HANDLE         HMODULE
#   define DYNLIB_LOAD( a )      LoadLibrary( a ) 
#   define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b ) 
#   define DYNLIB_UNLOAD( a )    !FreeLibrary( a ) 
#elif defined MOUCA_OS_LINUX
#   include <dlfcn.h>
#   define DYNLIB_HANDLE         void* 
#   define DYNLIB_LOAD( a )      dlopen( a, RTLD_LAZY ) 
#   define DYNLIB_GETSYM( a, b ) dlsym( a, b ) 
#   define DYNLIB_UNLOAD( a )    dlclose( a ) 
#endif 

namespace Core
{

using PlugInLoadingEntryPoint = PluginEntrySPtr (CALLBACK *)(void);

PlugInManager::PlugInArray::const_iterator PlugInManager::searchPlugIn(const PlugInManager::PlugInState& _instance) const
{
    auto itSearch = _loadedPlugins.cbegin();
    while(itSearch != _loadedPlugins.cend())
    {
        if(**itSearch == _instance)
            break;
        ++itSearch;
    }
    return itSearch;
}

PlugInEntrySPtr PlugInManager::loadDynamicLibrary(const Core::StringOS& strDynamicLibraryPath)
{
    BT_ASSERT(!strDynamicLibraryPath.empty());

    PlugInEntrySPtr pPlugInInformation;

    //Search if plugIn is already loaded
    auto itPlugIn = searchPlugIn(PlugInState(strDynamicLibraryPath));
    if(itPlugIn==_loadedPlugins.cend())
    {
        DYNLIB_HANDLE hGetProcIDDLL = DYNLIB_LOAD(strDynamicLibraryPath.c_str());
        if(!hGetProcIDDLL)
        {
            BT_THROW_ERROR_1(u8"BasicError", u8"DLLLoadingMissingFile", Core::convertToU8(strDynamicLibraryPath));
        }

        //Try to load entry point
        auto pLauncher = reinterpret_cast<PlugInLoadingEntryPoint>(DYNLIB_GETSYM(hGetProcIDDLL, "PlugInLoadingEntryPoint"));
        if(pLauncher==nullptr)
        {
            BT_THROW_ERROR_1(u8"BasicError", u8"DLLMissingEntryPointFile", std::to_string(GetLastError()));
        }

        auto pPlugInInstance = pLauncher();
        if(pPlugInInstance==nullptr)
        {
            BT_THROW_ERROR(u8"BasicError", u8"DLLCorruptionFile");
        }

        //Memorize PlugIn
        auto* completPlugIn = new PlugInState(hGetProcIDDLL, strDynamicLibraryPath, pPlugInInstance);
        _loadedPlugins.push_back(std::shared_ptr<PlugInState>(completPlugIn));
        
        pPlugInInformation = pPlugInInstance;
    }
    else
        pPlugInInformation = (*itPlugIn)->_plugInInstance;

    return pPlugInInformation;
}

void PlugInManager::release()
{
    for(auto& plugIn : _loadedPlugins)
    {
        BT_ASSERT(plugIn->_plugInInstance.use_count() <= 1); // DEV Issue: Need latest instance to guaranty memory security !

        // Clean instance
        plugIn->_plugInInstance.reset();
        
        // Remove DLL handle
        if(DYNLIB_UNLOAD((DYNLIB_HANDLE)plugIn->_hHandle))
        {
            BT_THROW_ERROR_1(u8"BasicError", u8"InvalidPathError", Core::convertToU8(plugIn->_name));
        }
    }
    _loadedPlugins.clear();
}

}