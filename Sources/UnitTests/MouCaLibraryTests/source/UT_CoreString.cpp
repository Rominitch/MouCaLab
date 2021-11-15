#include "Dependencies.h"

#include <LibCore/include/CoreString.h>

TEST(CoreString, constructor)
{
    Core::StringOS myString(L"ABCDE");

    ASSERT_EQ(L"ABCDE", myString);

    //Core::StringUTF8 myUTF8String(u8"ABCDE");
    //ASSERT_EQ("ABCDE", myUTF8String);

    Core::StringCPP myCppString("ABCDE");
    ASSERT_EQ("ABCDE", myCppString);
}

TEST(CoreString, os)
{
    /*
    Core::StringUTF8 string(u8"My String g�nial");

    Core::StringOS osString = Core::convertToOS(string);
    EXPECT_EQ(L"My String g�nial", osString);

    Core::StringUTF8 newString = Core::convertToU8(osString);
    EXPECT_EQ(string, newString);
    */

    Core::StringCPP string("My String g�nial");

    Core::StringOS osString = Core::convertToOS(string);
    EXPECT_EQ(L"My String g�nial", osString);

    Core::StringCPP newString = Core::convertToU8(osString);
    EXPECT_EQ(string, newString);
}