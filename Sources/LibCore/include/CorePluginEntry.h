/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    class PlugInEntry
    {
        public:
            virtual ~PlugInEntry() = default;

            virtual void setApplicationArguments(const int argc, const char** argv) = 0;

            virtual void initialize() = 0;
            virtual void release()    = 0;

        protected:
            PlugInEntry() = default;
    };

    using PlugInEntrySPtr = std::shared_ptr<PlugInEntry>;
    using PlugInEntryWPtr = std::weak_ptr<PlugInEntry>;
    using PlugInEntryUPtr = std::unique_ptr<PlugInEntry>;
}

#define DECLARE_PLUG_IN(classPlugIn)														\
extern "C" __declspec(dllexport) Core::PlugInEntry* CALLBACK PlugInLoadingEntryPoint()	    \
{																							\
    return new classPlugIn(); 											                    \
}                                                                                           \
