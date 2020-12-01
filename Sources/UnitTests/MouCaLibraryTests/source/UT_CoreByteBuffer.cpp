#include "Dependencies.h"

#include <LibCore/include/CoreFile.h>
#include <LibCore/include/CoreByteBuffer.h>

TEST(CoreByteBuffer, standardUse)
{
    Core::File output;
    output.setFileInfo(MouCaEnvironment::getOutputPath() / Core::StringOS(L"BTByteBuffer.txt"));

    Core::ByteBuffer buffer;
    EXPECT_EQ(0, buffer.size());

    const int val = 12;
    const std::vector<int> ints = { 17, 18, 19 };
    const std::pair<int, double> pairs = { 17, 18.5 };

    Core::ByteBuffer another;
    const int    a = 10;
    const float  b = 11.0f;
    const double c = -12.0f;
    another << a << b << c;

    const size_t buffersSize = 69 + 13;
    // Write
    {
        ASSERT_NO_THROW(buffer << val << u8"abcd" << 15.0f << 16.0);
        ASSERT_NO_THROW(buffer << Core::String("Hello"));
        ASSERT_NO_THROW(buffer << ints);
        ASSERT_NO_THROW(buffer << pairs);
        ASSERT_NO_THROW(buffer << another);

        EXPECT_EQ(buffersSize, buffer.size());

        ASSERT_NO_THROW(output.open(L"w"));
        ASSERT_NO_THROW(buffer.write(output));
        ASSERT_NO_THROW(output.close());
    }

    // Read
    {
        ASSERT_NO_THROW(output.open(L"r"));
        buffer.read(output, buffersSize);
        ASSERT_NO_THROW(output.close());

        EXPECT_EQ(buffersSize, buffer.size());

        int i;
        float f;
        double d;
        std::array<char, 5> str;
        std::vector<int> rints;
        std::pair<int, double> rpairs;
        Core::String btString;
        ASSERT_NO_THROW(buffer >> i >> str >> f >> d);
        ASSERT_NO_THROW(buffer >> btString);
        ASSERT_NO_THROW(buffer >> rints);
        ASSERT_NO_THROW(buffer >> rpairs);
        int    ar;
        float  br;
        double cr;
        ASSERT_NO_THROW(buffer >> ar >> br >> cr);

        EXPECT_EQ(val,      i);
        EXPECT_EQ(std::string(u8"abcd"), std::string(str.data()));
        EXPECT_EQ(15.0f,    f);
        EXPECT_EQ(16.0,     d);
        EXPECT_EQ(Core::String("Hello"), btString);
        EXPECT_EQ(ints,     rints);
        EXPECT_EQ(pairs,    rpairs);
        EXPECT_EQ(a,        ar);
        EXPECT_EQ(b,        br);
        EXPECT_EQ(c,        cr);
    }
}