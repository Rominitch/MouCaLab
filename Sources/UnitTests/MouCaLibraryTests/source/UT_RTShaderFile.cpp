#include "Dependancies.h"

#include <LibRT/include/RTShaderFile.h>

namespace RT 
{

TEST(RTShaderFile, getTrackedFile)
{
    Core::Path fileSource     = MouCaEnvironment::getInputPath() / L"libraries" / L"fragment.frag";
    Core::Path compiledSource = MouCaEnvironment::getOutputPath() / L"fragment.spv";

    ShaderFile shaderFile(compiledSource, RT::ShaderKind::Fragment);
    EXPECT_EQ(compiledSource, shaderFile.getTrackedFilename());

    ShaderFile tracked(compiledSource, RT::ShaderKind::Fragment, fileSource);
    EXPECT_EQ(fileSource.string(), tracked.getTrackedFilename().string());
}

// cppcheck-suppress syntaxError
TEST(RTShaderFile, compilation)
{
    // Valid compilation
    {
        Core::Path fileSource     = MouCaEnvironment::getInputPath() / L"libraries" / L"fragment.frag";
        Core::Path compiledSource = MouCaEnvironment::getOutputPath() / L"fragment.spv";

        ShaderFile shaderFile(compiledSource, RT::ShaderKind::Fragment, fileSource);

        // Call compilation
        ASSERT_NO_THROW(shaderFile.compile());

        // Re-entrancy
        ASSERT_NO_THROW(shaderFile.compile());
    }

    // Failed compilation
    {
        Core::Path fileSource     = MouCaEnvironment::getInputPath() / L"libraries" / "invalid.frag";
        Core::Path compiledSource = MouCaEnvironment::getOutputPath() / L"bugged.spv";

        ShaderFile shaderFile(compiledSource, RT::ShaderKind::Fragment, fileSource);

        // Call compilation
        ASSERT_ANY_THROW(shaderFile.compile());

        // Re-entrancy
        ASSERT_ANY_THROW(shaderFile.compile());
    }
}

}