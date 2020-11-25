/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CorePluginEntry.h>

class PlugInTest : public Core::PlugInEntry
{
    public:
        PlugInTest() = default;
        ~PlugInTest() override = default;

        void setApplicationArguments(const int argc, const char** argv) override
        {}

        void initialize() override;
        void release() override;
};