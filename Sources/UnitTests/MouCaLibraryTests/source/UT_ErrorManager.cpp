#include "Dependencies.h"

#include <LibXML/include/XMLParser.h>

#include <LibError/include/ErrorLibrary.h>
#include <LibError/include/ErrorPrinter.h>
#include <LibError/include/ErrorManager.h>

class ErrorManager : public ::testing::Test
{
    public:
        XML::XercesPlatform platform;
        XML::XercesParser   invalid;
        XML::XercesParser   demoLib;

        void SetUp() override
        {
            demoLib.setFileInfo( MouCaEnvironment::getInputPath() / L"libraries" / L"ErrorXML" / L"DemoLibrary.xml" );
        }

        void TearDown() override
        {}
};

TEST_F(ErrorManager, addErrorLibrary)
{
    Core::ErrorManager manager;

    ASSERT_ANY_THROW(manager.addErrorLibrary(invalid, "Invalid"));

    // Can load it first time
    ASSERT_NO_THROW(manager.addErrorLibrary(demoLib, "DemoLibrary"));
    // Re-entrance is not possible
    ASSERT_ANY_THROW(manager.addErrorLibrary(demoLib, "DemoLibrary"));

    {
        XML::XercesParser file;
        file.setFileInfo( MouCaEnvironment::getInputPath() / L"libraries" / L"ErrorXML" / L"unknowCountry.xml" );
        ASSERT_NO_THROW(manager.addErrorLibrary(file, "unknowCountry"));
    }
}

TEST_F(ErrorManager, invalidFiles)
{
    Core::ErrorManager manager;

    const std::array<Core::String, 3> files
    {
        "invalidBadName",
        "invalidMissingMessage",
        "invalidMissingSolution"
    };

    for (const auto& fileName : files)
    {
        XML::XercesParser file;
        file.setFileInfo(MouCaEnvironment::getInputPath() / L"libraries" / L"ErrorXML" / (fileName + ".xml"));

        ASSERT_ANY_THROW(manager.addErrorLibrary(file, fileName)) << fileName;
    }
}

TEST_F(ErrorManager, getError)
{
    Core::ErrorManager manager;

    // Without library registered
    {
        Core::ErrorData error;
        const Core::String result = error.getLibraryLabel() + " " + error.getErrorLabel();
        EXPECT_EQ( result, manager.getError(error) );
    }

    // With library registered
    {
        ASSERT_NO_THROW(manager.addErrorLibrary(demoLib, "DemoLibrary"));

        Core::ErrorData error("DemoLibrary", "Hello", __FILE__, __LINE__);
        EXPECT_EQ("Hello World\r\nHello Solution", manager.getError(error));
    }
}

struct TESTPrinter : public Core::ErrorPrinter
{
    Core::String message;

    void print(const Core::ErrorData& pError, const Core::ErrorLibrary* pLibrary) const override
    {
        Core::StringStream ssStream;
        const Core::ErrorDescription* pDescription = pLibrary->getDescription(pError.getErrorLabel());
        ssStream << pError.convertMessage(pDescription->getMessage()) << "\r\n" << pError.convertMessage(pDescription->getSolution());
        const_cast<TESTPrinter*>(this)->message = ssStream.str();
    }
};

TEST_F(ErrorManager, show)
{
    Core::ErrorManager manager;

    Core::Exception myException(Core::ErrorData("DemoLibrary", "Hello", __FILE__, __LINE__));

    // Valid printer
    {
        auto* printer = new TESTPrinter();
        Core::ErrorPrinterPtr printerPtr(printer);

        ASSERT_NO_THROW(manager.setPrinter(printerPtr));
        ASSERT_NO_THROW(manager.addErrorLibrary(demoLib, "DemoLibrary"));

        ASSERT_NO_THROW(manager.show(myException));

        EXPECT_EQ("Hello World\r\nHello Solution", printer->message);
    }
}
