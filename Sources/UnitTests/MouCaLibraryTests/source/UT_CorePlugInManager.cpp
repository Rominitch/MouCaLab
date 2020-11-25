#include "Dependancies.h"

#include "LibCore/include/CorePluginEntry.h"
#include "LibCore/include/CorePluginManager.h"

TEST(CorePluginManager, LoadUnexistingPlugIn)
{
    Core::PlugInManager manager;

    Core::PlugInEntrySPtr plugIn;
    EXPECT_THROW(plugIn = manager.loadDynamicLibrary(L"invalidDll.dll"), Core::Exception);
    EXPECT_TRUE(plugIn == NULL);
}

TEST(CorePluginManager, LoadExistingPlugIn)
{
    Core::PlugInManager manager;

    // Load new plugin
    Core::PlugInEntrySPtr plugIn;
    {
        EXPECT_NO_THROW(plugIn = manager.loadDynamicLibrary(MouCaEnvironment::getWorkingPath() / L"MouCaPlugInTest.dll"));
        EXPECT_TRUE(plugIn != NULL);

        ASSERT_NO_THROW(plugIn->setApplicationArguments(0, NULL));

        ASSERT_NO_THROW(plugIn->initialize());
    }

    // Get existing plugin
    {
        Core::PlugInEntrySPtr plugInReloaded;
        EXPECT_NO_THROW(plugInReloaded = manager.loadDynamicLibrary(MouCaEnvironment::getWorkingPath() / L"MouCaPlugInTest.dll"));
        EXPECT_EQ(plugIn, plugInReloaded);
    }
}