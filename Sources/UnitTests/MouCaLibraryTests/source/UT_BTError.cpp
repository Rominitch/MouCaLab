#include "Dependancies.h"

#include <LibCore/include/BTError.h>

TEST(BTErrorData, createDefault)
{
    Core::ErrorData error;

    EXPECT_EQ(u8"BasicError",   error.getLibraryLabel());
    EXPECT_EQ(u8"UnknownError", error.getErrorLabel());
    EXPECT_EQ("",               error.getFile());
    EXPECT_EQ(0,                error.getLine());
    EXPECT_EQ(0,                error.getNbParameters());
}

TEST(BTErrorData, createParam)
{
    const Core::String mylib(u8"DemoLib");
    const Core::String myerror(u8"DemoError");
    const Core::String myfile("c:\\pouet\\error.cpp");
    const int myLine = 12345;
    const Core::String myParameter1(u8"parameter1");

    Core::ErrorData error1(mylib, myerror, myfile, myLine);

    EXPECT_EQ(mylib,        error1.getLibraryLabel());
    EXPECT_EQ(myerror,      error1.getErrorLabel());
    EXPECT_EQ(myfile,       error1.getFile());
    EXPECT_EQ(myLine,       error1.getLine());
    EXPECT_EQ(0,            error1.getNbParameters());

    Core::ErrorData error(mylib, myerror, myfile, myLine);
    error << myParameter1;

    EXPECT_EQ(mylib,        error.getLibraryLabel());
    EXPECT_EQ(myerror,      error.getErrorLabel());
    EXPECT_EQ(myfile,       error.getFile());
    EXPECT_EQ(myLine,       error.getLine());
    EXPECT_EQ(1,            error.getNbParameters());
    EXPECT_EQ(myParameter1, error.getParameters(0));
    
    Core::ErrorData copy(error);
    EXPECT_EQ(mylib,        copy.getLibraryLabel());
    EXPECT_EQ(myerror,      copy.getErrorLabel());
    EXPECT_EQ(myfile,       copy.getFile());
    EXPECT_EQ(myLine,       copy.getLine());
    EXPECT_EQ(1,            copy.getNbParameters());
    EXPECT_EQ(myParameter1, copy.getParameters(0));
}

TEST(BTErrorData, convertMessage)
{
    const Core::String mylib(u8"DemoLib");
    const Core::String myerror(u8"DemoError");
    const Core::String myParameter1(u8"parameter1");

    const Core::String message(u8"%P0");

    // Parameters tests
    {
        Core::ErrorData error(mylib, myerror, __FILE__, __LINE__);
        EXPECT_EQ(message, error.convertMessage(message));
    }

    {
        Core::ErrorData error(mylib, myerror, __FILE__, __LINE__);
        error << myParameter1;
        EXPECT_EQ(myParameter1, error.convertMessage(message));
    
        EXPECT_EQ(myParameter1+u8"-%P1", error.convertMessage(u8"%P0-%P1"));
    }

    // End of line
    {
        Core::ErrorData error(mylib, myerror, __FILE__, __LINE__);
        EXPECT_EQ(u8"\r\n", error.convertMessage(u8"\n"));

        EXPECT_EQ(u8"\r\nabc\r\nefgh\tijkl\r\n", error.convertMessage(u8"\nabc\nefgh\tijkl\n"));
    }
}