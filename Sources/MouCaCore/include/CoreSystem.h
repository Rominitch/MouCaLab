/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <MouCaCore/include/Core.h>

#include <LibCore/include/CorePluginManager.h>
#include <LibCore/include/CoreThreadPools.h>

#include <LibError/include/ErrorManager.h>

#include <LibXML/include/XML.h>

#include <MouCaCore/include/LoaderManager.h>
#include <MouCaCore/include/ResourceManager.h>

namespace Core
{
    class Locale; 
    using LocaleSPtr = std::shared_ptr<Locale>;
}

namespace MouCaCore
{
    ///	\class	CoreSystem
    ///	\brief	Main system: allow to load file/manage thread and all OS problematics
    class CoreSystem : public ICoreSystem
    {
        MOUCA_NOCOPY(CoreSystem);

        public:
            CoreSystem();
            ~CoreSystem() override = default;

            Core::PlugInManager& getPlugInManager() override
            {
                return _pluginManager;
            }
            
            Core::ThreadPools& getThreadPools() override
            {
                return _threadPool;
            }

            Core::IErrorManager& getExceptionManager() override
            {
                return _errorManager;
            }

            ResourceManager& getResourceManager() override
            {
                return *_resourceManager;
            }

            LoaderManager& getLoaderManager() override
            {
                return _loaderManager;
            }

            const Core::Locale& getLocale() const override
            {
                return *_locale;
            }
            
            void printException(const Core::Exception& exception) const override;

        protected:
            XML::PlatformUPtr     _xmlPlatform;
            Core::PlugInManager   _pluginManager;
            Core::ThreadPools	  _threadPool;
            ResourceManagerSPtr   _resourceManager;
            LoaderManager         _loaderManager;

            Core::LocaleSPtr      _locale;
            Core::ErrorManager    _errorManager;
    };

    using CoreSystemUPtr = std::unique_ptr<CoreSystem>;
}