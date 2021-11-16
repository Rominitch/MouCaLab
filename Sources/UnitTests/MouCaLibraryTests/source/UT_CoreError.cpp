#include "Dependencies.h"

#include <LibCore/include/CoreError.h>

TEST(CoreErrorData, createDefault)
{
    Core::ErrorData error;

    EXPECT_EQ("BasicError",   error.getLibraryLabel());
    EXPECT_EQ("UnknownError", error.getErrorLabel());
    EXPECT_EQ(0,              error.getNbParameters());
}

TEST(CoreErrorData, createParam)
{
    const auto thisFile = std::source_location::current();
    const Core::String mylib("DemoLib");
    const Core::String myerror("DemoError");
    const Core::String myParameter1("parameter1");

    Core::ErrorData error1(mylib, myerror);

    EXPECT_EQ(mylib,        error1.getLibraryLabel());
    EXPECT_EQ(myerror,      error1.getErrorLabel());
    EXPECT_EQ(thisFile.file_name(), error1.getSourceLocation().file_name());
    EXPECT_EQ(0,            error1.getNbParameters());

    Core::ErrorData error(mylib, myerror);
    error << myParameter1;

    EXPECT_EQ(mylib,        error.getLibraryLabel());
    EXPECT_EQ(myerror,      error.getErrorLabel());
    EXPECT_EQ(thisFile.file_name(), error.getSourceLocation().file_name());
    EXPECT_EQ(1,            error.getNbParameters());
    EXPECT_EQ(myParameter1, error.getParameters(0));
    
    Core::ErrorData copy(error);
    EXPECT_EQ(mylib,        copy.getLibraryLabel());
    EXPECT_EQ(myerror,      copy.getErrorLabel());
    EXPECT_EQ(thisFile.file_name(), copy.getSourceLocation().file_name());
    EXPECT_EQ(1,            copy.getNbParameters());
    EXPECT_EQ(myParameter1, copy.getParameters(0));
}

TEST(CoreErrorData, convertMessage)
{
    const Core::String mylib("DemoLib");
    const Core::String myerror("DemoError");
    const Core::String myParameter1("parameter1");

    const Core::String message("%P0");

    // Parameters tests
    {
        Core::ErrorData error(mylib, myerror);
        EXPECT_EQ(message, error.convertMessage(message));
    }

    {
        Core::ErrorData error(mylib, myerror);
        error << myParameter1;
        EXPECT_EQ(myParameter1, error.convertMessage(message));
    
        EXPECT_EQ(myParameter1+"-%P1", error.convertMessage("%P0-%P1"));
    }

    // End of line
    {
        Core::ErrorData error(mylib, myerror);
        EXPECT_EQ("\r\n", error.convertMessage("\n"));

        EXPECT_EQ("\r\nabc\r\nefgh\tijkl\r\n", error.convertMessage("\nabc\nefgh\tijkl\n"));
    }
}