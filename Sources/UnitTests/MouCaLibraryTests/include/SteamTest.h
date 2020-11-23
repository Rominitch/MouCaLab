#pragma once

#include <LibSteam/include/Platform.h>

class MouCaSteam : public ::testing::Test
{
    public:
        static void SetUpTestSuite()
        {}

        static void TearDownTestSuite()
        {}

        void SetUp() override;

        void TearDown() override;

    protected:
        Steam::Platform _platform;
};