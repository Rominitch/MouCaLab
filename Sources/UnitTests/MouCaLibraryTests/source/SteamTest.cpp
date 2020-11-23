#include "Dependancies.h"

#include "include/SteamTest.h"

void MouCaSteam::SetUp()
{
    ASSERT_NO_THROW(_platform.initialize());
}

void MouCaSteam::TearDown()
{
    if (!_platform.isNull())
    {
        ASSERT_NO_THROW(_platform.release());
    }
}