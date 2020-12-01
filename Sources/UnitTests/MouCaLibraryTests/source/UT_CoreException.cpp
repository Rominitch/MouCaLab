#include "Dependencies.h"

#include <LibCore/include/CoreError.h>
#include <LibCore/include/CoreException.h>

TEST(CoreException, create)
{
    const Core::ErrorData error;
    Core::Exception exception(error);

    ASSERT_EQ(1, exception.getNbErrors());
    
    const Core::ErrorData& savedError = exception.read(0);
    EXPECT_EQ(error.getLibraryLabel(),  savedError.getLibraryLabel());
    EXPECT_EQ(error.getErrorLabel(),    savedError.getErrorLabel());
    EXPECT_EQ(error.getFile(),          savedError.getFile());
    EXPECT_EQ(error.getLine(),          savedError.getLine());
    EXPECT_EQ(error.getNbParameters(),  savedError.getNbParameters());
    
    Core::ErrorData error2(u8"MyLib", u8"MyError", __FILE__, __LINE__);
    ASSERT_NO_THROW(exception.addCatchedError(error2));
    ASSERT_EQ(2, exception.getNbErrors());
    
    const Core::ErrorData& savedError2 = exception.read(0);
    EXPECT_EQ(error2.getLibraryLabel(),  savedError2.getLibraryLabel());
    EXPECT_EQ(error2.getErrorLabel(),    savedError2.getErrorLabel());
    EXPECT_EQ(error2.getFile(),          savedError2.getFile());
    EXPECT_EQ(error2.getLine(),          savedError2.getLine());
    EXPECT_EQ(error2.getNbParameters(),  savedError2.getNbParameters());
}