#include "Dependancies.h"

#include <LibCore/include/BTFile.h>
#include <LibCore/include/BTResource.h>

TEST(BTResource, standardUse)
{
    const Core::Path file(L"myFile.txt");
    Core::File resource(file, Core::Resource::Mode::AutoRefreshCritical);

    EXPECT_EQ(file, resource.getFilename());
    EXPECT_TRUE(resource.isMode(Core::Resource::Mode::Critical));
    EXPECT_TRUE(resource.isMode(Core::Resource::Mode::AutoRefresh));
    EXPECT_FALSE(resource.isMode(Core::Resource::Mode::Temporary));
}