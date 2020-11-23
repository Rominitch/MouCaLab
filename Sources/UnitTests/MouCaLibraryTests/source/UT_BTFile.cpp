#include "Dependancies.h"

#include <LibCore/include/BTFile.h>
#include <LibCore/include/BTString.h>

TEST(BTFile, open)
{
    Core::Path string( MouCaEnvironment::getInputPath() / L"libraries" / L"日.txt");

    {
        Core::File myfile;
        ASSERT_NO_THROW(myfile.setFileInfo(string));
        ASSERT_NO_THROW(myfile.open(L"r"));
        ASSERT_NO_THROW(myfile.close());

        EXPECT_EQ(string, myfile.getFilePath());
    }

    {
        Core::File myfile(string);
        ASSERT_NO_THROW(myfile.open(L"r")) << u8"File: " << string.string();
    }
}

TEST(BTFile, errorChecking)
{
    // Bad open
    {
        Core::Path string(MouCaEnvironment::getInputPath() / L"libraries" / L"UnknownFileName.txt");
        Core::File myfile;

        ASSERT_NO_THROW(myfile.setFileInfo(string));
        ASSERT_ANY_THROW(myfile.open(L"r")) << u8"File: " << string.string();
    }

    /*
    Core::StringOS string(MouCaEnvironment::getInputPath() + L"MyString.txt");
    Core::File myfile;
    {
        ASSERT_NO_THROW(myfile.initialize(string));
        ASSERT_NO_THROW(myfile.open(L"r"));

        EXPECT_ANY_THROW(myfile.setFilePosition(12345));
        EXPECT_ANY_THROW(myfile.getFilePosition());

        ASSERT_NO_THROW(myfile.close());
    }
    */
}

TEST(BTFile, readString)
{
    Core::Path string( MouCaEnvironment::getInputPath() / L"libraries" / L"MyString.txt");
    Core::File myfile;

    ASSERT_TRUE(Core::File::isExist(string));
    
    ASSERT_NO_THROW(myfile.setFileInfo(string));
    ASSERT_TRUE(myfile.isExist());
    EXPECT_FALSE(myfile.isOpened());

    ASSERT_NO_THROW(myfile.open(L"r")) << u8"File: " << string.string();
    EXPECT_TRUE(myfile.isOpened());

    EXPECT_EQ("Juste un petit texte", myfile.extractString());

    ASSERT_NO_THROW(myfile.close());
}

TEST(BTFile, readUTF8)
{
    Core::Path string( MouCaEnvironment::getInputPath() / L"libraries" / L"MyUTF8.txt");
    
    Core::File myfile;
    ASSERT_NO_THROW(myfile.setFileInfo(string));
    ASSERT_NO_THROW(myfile.open(L"r"));

    // UTF8 sans BOM
    EXPECT_EQ(Core::convertToU8(L"\u7530\u4E2D\u3002"), myfile.extractUTF8());

    ASSERT_NO_THROW(myfile.close());
}

TEST(BTFile, write)
{
    Core::Path string(MouCaEnvironment::getOutputPath() / L"MyText.txt");

    Core::File myfile;
    ASSERT_NO_THROW(myfile.setFileInfo(string));
    ASSERT_NO_THROW(myfile.open(L"w+")) << u8"File: " << string.string();
    const std::string text("Juste un petit texte");
    
    ASSERT_NO_THROW(myfile.write(text.c_str(), sizeof(char)*text.size()));
    ASSERT_NO_THROW(myfile.write(19, "o", sizeof(char)));

    EXPECT_EQ("Juste un petit texto", myfile.extractString());

    ASSERT_NO_THROW(myfile.close());
}

TEST(BTFile, writeText)
{
    Core::Path string(MouCaEnvironment::getOutputPath() / L"MyTextFormatted.txt");

    {
        Core::File myfile;
        ASSERT_NO_THROW(myfile.setFileInfo(string));
        ASSERT_NO_THROW(myfile.open(L"w+")) << u8"File: " << string.string();
        ASSERT_NO_THROW(myfile.writeText("Hello World: %d", 42));
        ASSERT_NO_THROW(myfile.close());
    }

    {
        Core::File myfile;
        ASSERT_NO_THROW(myfile.setFileInfo(string));
        ASSERT_NO_THROW(myfile.open(L"r")) << u8"File: " << string.string();
        EXPECT_EQ("Hello World: 42", myfile.extractString());
        ASSERT_NO_THROW(myfile.close());
    }
}