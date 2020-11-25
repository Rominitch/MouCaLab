/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CorePluginManager.h>
#include <LibCore/include/CoreThreadPools.h>

#include <LibError/include/ErrorManager.h>

#include <LibXML/include/XMLParser.h>

#include "MouCaCore/include/LoaderManager.h"
#include "MouCaCore/include/ResourceManager.h"

namespace Core
{
    class Locale; 
    using LocaleSPtr = std::shared_ptr<Locale>;
}

namespace MouCaCore
{
    ///	\class	CoreSystem
    ///	\brief	Main system: allow to load file/manage thread and all OS problematics
    class CoreSystem
    {
        MOUCA_NOCOPY_NOMOVE(CoreSystem);

        public:
            CoreSystem();
            ~CoreSystem() = default;

            Core::PlugInManager& getPlugInManager()
            {
                return _pluginManager;
            }
            
            Core::ThreadPools& getThreadPools()
            {
                return _threadPool;
            }

            Core::ErrorManager& getExceptionManager()
            {
                return _errorManager;
            }

            ResourceManager& getResourceManager()
            {
                return *_resourceManager;
            }

            LoaderManager& getLoaderManager()
            {
                return _loaderManager;
            }

            const Core::Locale& getLocale() const
            {
                return *_locale;
            }
            
            void printException(const Core::Exception& exception) const;

        protected:
            XML::Platform           _xmlPlatform;
            Core::PlugInManager     _pluginManager;
            Core::ThreadPools	    _threadPool;
            ResourceManagerSPtr     _resourceManager;
            LoaderManager           _loaderManager;

            Core::LocaleSPtr      _locale;
            Core::ErrorManager    _errorManager;
    };

    using CoreSystemUPtr = std::unique_ptr<CoreSystem>;
}