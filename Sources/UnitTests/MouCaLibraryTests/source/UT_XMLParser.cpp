#include "Dependencies.h"

#include <LibXML/include/XMLParser.h>

TEST(XMLParser, platform)
{
    XML::Platform platform;

    // Try to create both parser.
    {
        XML::Platform platform2;
    }
}

TEST(XMLParser, noPlatform)
{
    XML::XercesParser parser;

    const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries" / L"DemoXML.xml";
    ASSERT_ANY_THROW(parser.openXMLFile(filenameXML));
}

TEST(XMLParser, OpenError)
{
    XML::Platform platform;
    {
        XML::XercesParser parser;

        const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries"/ L"Invalid.xml";
        ASSERT_ANY_THROW(parser.openXMLFile(filenameXML));
    }
}

TEST(XMLParser, multiOpen)
{
    XML::Platform platform;

    {
        XML::XercesParser parser;

        // Try to create both parser.
        {
            XML::XercesParser parser2;
        }

        const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries"/ L"DemoXML.xml";
        ASSERT_NO_THROW(parser.openXMLFile(filenameXML));

        XML::NodeUPtr languageNode;
        ASSERT_NO_THROW(languageNode = parser.searchNode(u8"Language", u8"country", u8"FR"));

        ASSERT_NO_THROW(parser.release());
    }
}

TEST(XMLParser, searchNode)
{
    XML::Platform platform;

    {
        XML::XercesParser parser;

        const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries" / L"DemoXML.xml";
        ASSERT_NO_THROW(parser.openXMLFile(filenameXML));

        XML::NodeUPtr languageNode;
        ASSERT_NO_THROW(languageNode = parser.searchNode(u8"Language", u8"country", u8"FR"));

        ASSERT_ANY_THROW(languageNode = parser.searchNode(u8"Language", u8"country", u8"EN"));

        ASSERT_NO_THROW(parser.release());
    }
}

TEST(XMLParser, readNode)
{
    XML::Platform platform;

    {
        XML::XercesParser parser;

        const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries" / L"DemoXML.xml";
        ASSERT_NO_THROW(parser.openXMLFile(filenameXML));

        XML::NodeUPtr languageNode;
        ASSERT_NO_THROW(languageNode = parser.searchNode(u8"Language", u8"country", u8"FR"));

        EXPECT_TRUE(static_cast<XML::XercesNode*>(languageNode.get())->getNode() != nullptr);
        
        parser.pushNode(*languageNode);
        
        XML::NodeUPtr codeNode;
        ASSERT_NO_THROW(codeNode = parser.searchNode(u8"Error", u8"name", u8"MyCode"));

        Core::String sCode;
        ASSERT_ANY_THROW(codeNode->getAttribute(u8"invalid", sCode));
        EXPECT_FALSE(codeNode->hasAttribute(u8"invalid"));
        
        unsigned int code;
        ASSERT_NO_THROW(codeNode->getAttribute(u8"code", code));
        EXPECT_TRUE(codeNode->hasAttribute(u8"code"));
        EXPECT_EQ(1ul, code);
        ASSERT_ANY_THROW(codeNode->getAttribute(u8"invalid", code));
        int icode;
        ASSERT_NO_THROW(codeNode->getAttribute(u8"code", icode));
        EXPECT_EQ(1, icode);
        ASSERT_ANY_THROW(codeNode->getAttribute(u8"invalid", icode));

        uint64_t ui64Code = 0ull;
        ASSERT_NO_THROW(codeNode->getAttribute(u8"code", ui64Code));
        EXPECT_EQ(1ull, ui64Code);
        ASSERT_ANY_THROW(codeNode->getAttribute(u8"invalid", ui64Code));
        int64_t i64Code = 0ll;
        ASSERT_NO_THROW(codeNode->getAttribute(u8"code", i64Code));
        EXPECT_EQ(1ll, i64Code);
        ASSERT_ANY_THROW(codeNode->getAttribute(u8"invalid", i64Code));

        float fCode;
        ASSERT_NO_THROW(codeNode->getAttribute(u8"fcode", fCode));
        EXPECT_EQ(2.122f, fCode);
        ASSERT_ANY_THROW(codeNode->getAttribute(u8"invalid", fCode));

        parser.pushNode(*codeNode);
        XML::ResultUPtr result;
        ASSERT_NO_THROW(result = parser.getNode(u8"Message"));

        Core::String value;
        ASSERT_NO_THROW(result->getNode(0)->getValue(value));
        EXPECT_EQ(u8"Hello World", value);

        parser.popNode();

        parser.popNode();

        ASSERT_NO_THROW(parser.release());
    }
}

TEST(XMLParser, autoParser)
{
    XML::Platform platform;

    {
        XML::XercesParser parser;

        const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries" / L"DemoXML.xml";
        ASSERT_NO_THROW(parser.openXMLFile(filenameXML));

        XML::NodeUPtr languageNode;
        ASSERT_NO_THROW(languageNode = parser.searchNode(u8"Language", u8"country", u8"FR"));

        EXPECT_TRUE(static_cast<XML::XercesNode*>(languageNode.get())->getNode() != nullptr);

        {
            auto p = parser.autoPushNode(*languageNode);

            XML::NodeUPtr codeNode;
            ASSERT_NO_THROW(codeNode = parser.searchNode(u8"Error", u8"name", u8"MyCode"));
        }

        ASSERT_NO_THROW(parser.release());
    }
}

TEST(XMLParser, searchNodeFrom)
{
    XML::Platform platform;

    {
        XML::XercesParser parser;

        const Core::Path filenameXML = MouCaEnvironment::getInputPath() / L"libraries" / L"DemoXML.xml";
        ASSERT_NO_THROW(parser.openXMLFile(filenameXML));

        XML::XercesNode emptyNode;
        XML::NodeUPtr libNode;
        ASSERT_NO_THROW(libNode = parser.searchNodeFrom(emptyNode, u8"ErrorLibrary", u8"name", u8"DemoLibrary2"));
        ASSERT_FALSE(libNode->isNull());

        XML::NodeUPtr errorNode;
        ASSERT_NO_THROW(errorNode = parser.searchNodeFrom(*libNode, u8"Error", u8"name", u8"Hello"));
        ASSERT_FALSE(errorNode->isNull());

        ASSERT_ANY_THROW(errorNode = parser.searchNodeFrom(*libNode, u8"Error", u8"name", u8"MyCode"));

        ASSERT_NO_THROW(parser.release());
    }
}