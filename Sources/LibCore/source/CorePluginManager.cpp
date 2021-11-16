/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

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

using PlugInLoadingEntryPoint = PlugInEntry* (CALLBACK *)(void);

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

PlugInEntrySPtr PlugInManager::loadDynamicLibrary(const Path& dynamicLibraryPath)
{
    MouCa::assertion(!dynamicLibraryPath.empty());

    PlugInEntrySPtr pPlugInInformation;

    //Search if plugIn is already loaded
    auto itPlugIn = searchPlugIn(PlugInState(dynamicLibraryPath));
    if(itPlugIn==_loadedPlugins.cend())
    {
        DYNLIB_HANDLE hGetProcIDDLL = DYNLIB_LOAD(dynamicLibraryPath.c_str());
        if(!hGetProcIDDLL)
        {
            throw Core::Exception(Core::ErrorData("BasicError", "DLLLoadingMissingFile") << dynamicLibraryPath.string());
        }

        //Try to load entry point
        auto pLauncher = reinterpret_cast<PlugInLoadingEntryPoint>(DYNLIB_GETSYM(hGetProcIDDLL, "PlugInLoadingEntryPoint"));
        if(pLauncher==nullptr)
        {
            throw Core::Exception(Core::ErrorData("BasicError", "DLLMissingEntryPointFile") << std::to_string(GetLastError()));
        }

        auto pPlugInInstance = pLauncher();
        if(pPlugInInstance==nullptr)
        {
            throw Core::Exception(Core::ErrorData("BasicError", "DLLCorruptionFile"));
        }

        pPlugInInformation = PluginEntrySPtr(pPlugInInstance);

        //Memorize PlugIn
        auto* completPlugIn = new PlugInState(hGetProcIDDLL, dynamicLibraryPath, pPlugInInformation);
        _loadedPlugins.push_back(std::shared_ptr<PlugInState>(completPlugIn));
    }
    else
        pPlugInInformation = (*itPlugIn)->_plugInInstance;

    return pPlugInInformation;
}

void PlugInManager::release()
{
    for(auto& plugIn : _loadedPlugins)
    {
        MouCa::assertion(plugIn->_plugInInstance.use_count() <= 1); // DEV Issue: Need latest instance to guaranty memory security !

        // Clean instance
        plugIn->_plugInInstance.reset();
        
        // Remove DLL handle
        if(DYNLIB_UNLOAD((DYNLIB_HANDLE)plugIn->_hHandle))
        {
            throw Core::Exception(Core::ErrorData("BasicError", "InvalidPathError") << Core::convertToU8(plugIn->_name));
        }
    }
    _loadedPlugins.clear();
}

}