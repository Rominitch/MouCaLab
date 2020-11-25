/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    class PluginEntry;
    using PluginEntrySPtr = std::shared_ptr<PluginEntry>;

    class PlugInManager : public std::enable_shared_from_this<PlugInManager>
    {
        protected:
            class PlugInState final
            {
                public:
                    void*			_hHandle;
                    Core::StringOS	_name;
                    PluginEntrySPtr _plugInInstance;
            
                    PlugInState(const std::wstring& strName):
                    _hHandle(nullptr), _name(strName)
                    {}

                    PlugInState(void* hHandle, const std::wstring& strName, PluginEntrySPtr pInstance):
                    _hHandle(hHandle), _name(strName), _plugInInstance(pInstance)
                    {}

                    ~PlugInState() = default;

                    bool operator==(const PlugInState& _instance) const
                    {
                        return _name == _instance._name;
                    }
            };
    
            using PlugInArray = std::vector<std::shared_ptr<PlugInState>>;
            PlugInArray _loadedPlugins;

            void release();

            PlugInArray::const_iterator searchPlugIn(const PlugInState& _instance) const;

        public:
            PlugInManager() = default;

            ~PlugInManager()
            {
                release();
            }

            PlugInEntrySPtr loadDynamicLibrary(const Core::StringOS& strDynamicLibraryPath);
    };

};