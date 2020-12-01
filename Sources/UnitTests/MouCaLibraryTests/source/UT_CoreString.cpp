#include "Dependencies.h"

#include <LibCore/include/CoreString.h>

TEST(CoreString, constructor)
{
    Core::StringOS myString(L"ABCDE");

    ASSERT_EQ(L"ABCDE", myString);

    Core::StringUTF8 myUTF8String(u8"ABCDE");
    ASSERT_EQ(u8"ABCDE", myUTF8String);
}

TEST(CoreString, os)
{
    Core::StringUTF8 string(u8"My String génial");

    Core::StringOS osString = Core::convertToOS(string);
    EXPECT_EQ(L"My String génial", osString);

    Core::StringUTF8 newString = Core::convertToU8(osString);
    EXPECT_EQ(string, newString);
}