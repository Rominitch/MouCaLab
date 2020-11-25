/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    class PluginEntry
    {
        public:
            virtual ~PluginEntry() = default;

            virtual void setApplicationArguments(const int argc, const char** argv) = 0;

            virtual void initialize() = 0;
            virtual void release()    = 0;

        protected:
            PluginEntry() = default;
    };

    using PlugInEntryPtr = PluginEntry*;
    using PlugInEntrySPtr = std::shared_ptr<PluginEntry>;
    using PlugInEntryWPtr = std::weak_ptr<PluginEntry>;
    using PlugInEntryUPtr = std::unique_ptr<PluginEntry>;
}

#define DECLARE_PLUG_IN(classPlugIn)														\
                                                                                            \
static classPlugIn* g_instance = NULL;                                                      \
                                                                                            \
extern "C" __declspec(dllexport) Core::PlugInEntryPtr CALLBACK PlugInLoadingEntryPoint()	\
{																							\
    g_instance = new classPlugIn();															\
    return g_instance;																	    \
}                                                                                           \
