#include "Dependancies.h"

#include <LibCore/include/CoreIdentifier.h>

TEST(CoreIdentifier, build)
{
    const Core::String idName0("idName0");
    const Core::String idName1("idName1");

    Core::Identifier id0;
    EXPECT_EQ(Core::String(), id0.getName());
    id0.setName(idName0);
    EXPECT_EQ(idName0, id0.getName());
    EXPECT_LE(0, id0.getGID());             // Application can allocate more ID so no idea of real value

    Core::Identifier id1;
    id1.setName(idName1);
    EXPECT_EQ(idName1, id1.getName());
    EXPECT_LE(1, id1.getGID());             // Application can allocate more ID so no idea of real value
    EXPECT_LT(id0.getGID(), id1.getGID());  // Just know order
}
