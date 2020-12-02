#include "Dependencies.h"

#include "include/SteamTest.h"

void MouCaSteam::SetUp()
{
    {
        Core::File steamAppId("steam_appID.txt");
        steamAppId.open(L"w");
        steamAppId.writeText("480");
        steamAppId.close();
    }

    ASSERT_NO_THROW(_platform.initialize()) << "Steam must be launched before gtest";
}

void MouCaSteam::TearDown()
{
    if (!_platform.isNull())
    {
        ASSERT_NO_THROW(_platform.release());
    }
}