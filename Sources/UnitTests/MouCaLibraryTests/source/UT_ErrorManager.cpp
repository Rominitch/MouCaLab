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

    ASSERT_ANY_THROW(manager.addErrorLibrary(invalid, u8"Invalid"));

    // Can load it first time
    ASSERT_NO_THROW(manager.addErrorLibrary(demoLib, u8"DemoLibrary"));
    // Re-entrance is not possible
    ASSERT_ANY_THROW(manager.addErrorLibrary(demoLib, u8"DemoLibrary"));

    {
        XML::XercesParser file;
        file.setFileInfo( MouCaEnvironment::getInputPath() / L"libraries" / L"ErrorXML" / L"unknowCountry.xml" );
        ASSERT_NO_THROW(manager.addErrorLibrary(file, u8"unknowCountry"));
    }
}

TEST_F(ErrorManager, invalidFiles)
{
    Core::ErrorManager manager;

    const std::array<Core::String, 3> files
    {
        u8"invalidBadName",
        u8"invalidMissingMessage",
        u8"invalidMissingSolution"
    };

    for (const auto& fileName : files)
    {
        XML::XercesParser file;
        file.setFileInfo(MouCaEnvironment::getInputPath() / L"libraries" / L"ErrorXML" / (fileName + u8".xml"));

        ASSERT_ANY_THROW(manager.addErrorLibrary(file, fileName)) << fileName;
    }
}

TEST_F(ErrorManager, getError)
{
    Core::ErrorManager manager;

    // Without library registered
    {
        Core::ErrorData error;
        const Core::String result = error.getLibraryLabel() + u8" " + error.getErrorLabel();
        EXPECT_EQ( result, manager.getError(error) );
    }

    // With library registered
    {
        ASSERT_NO_THROW(manager.addErrorLibrary(demoLib, u8"DemoLibrary"));

        Core::ErrorData error(u8"DemoLibrary", u8"Hello", __FILE__, __LINE__);
        EXPECT_EQ(u8"Hello World\r\nHello Solution", manager.getError(error));
    }
}

struct TESTPrinter : public Core::ErrorPrinter
{
    Core::String message;

    void print(const Core::ErrorData& pError, const Core::ErrorLibrary* pLibrary) const override
    {
        Core::StringStream ssStream;
        const Core::ErrorDescription* pDescription = pLibrary->getDescription(pError.getErrorLabel());
        ssStream << pError.convertMessage(pDescription->getMessage()) << u8"\r\n" << pError.convertMessage(pDescription->getSolution());
        const_cast<TESTPrinter*>(this)->message = ssStream.str();
    }
};

TEST_F(ErrorManager, show)
{
    Core::ErrorManager manager;

    Core::Exception myException(Core::ErrorData(u8"DemoLibrary", u8"Hello", __FILE__, __LINE__));

    // Valid printer
    {
        auto* printer = new TESTPrinter();
        Core::ErrorPrinterPtr printerPtr(printer);

        ASSERT_NO_THROW(manager.setPrinter(printerPtr));
        ASSERT_NO_THROW(manager.addErrorLibrary(demoLib, u8"DemoLibrary"));

        ASSERT_NO_THROW(manager.show(myException));

        EXPECT_EQ(u8"Hello World\r\nHello Solution", printer->message);
    }
}
