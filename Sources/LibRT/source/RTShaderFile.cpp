/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibCore/include/CoreProcess.h"
#include "LibCore/include/CoreStandardOS.h"

#include "LibRT/include/RTShaderFile.h"

namespace RT
{

ShaderFile::ShaderFile(const Core::Path& compiledPath, const ShaderKind kind, const Core::Path& filepath) :
Core::File(compiledPath, filepath.empty() ? Core::Resource::Mode::Critical : Core::Resource::Mode::AutoRefreshCritical), _kind(kind), _sourceFilepath(filepath)
{
    MOUCA_PRE_CONDITION(_sourceFilepath.empty() || std::filesystem::exists(_sourceFilepath) ); // DEV Issue: Need valid file if wanted.
    MOUCA_ASSERT_BETWEEN(_kind, RT::ShaderKind::Vertex, RT::ShaderKind::NbShaders);

    MOUCA_POST_CONDITION(!isNull() && !isLoaded());
}

ShaderFile::~ShaderFile()
{
    MOUCA_POST_CONDITION(!isLoaded());
}

void ShaderFile::compile()
{
    MOUCA_PRE_CONDITION( !_sourceFilepath.empty() ); // DEV Issue: Need valid source file: API is consider like disabled.

    const Core::Path executable = Core::Path(Core::getEnvironmentVariable("VULKAN_SDK")) / u8"bin" / u8"glslangValidator.exe";

    // Prepare task
    Core::Process processCompile(executable);
    Core::Process::Arguments arguments;
    arguments.emplace_back(L"-H");
    arguments.emplace_back(L"-V");
    arguments.emplace_back(L"-o");
    arguments.emplace_back(_filename);
    arguments.emplace_back(_sourceFilepath);

    // Execute
    processCompile.execute(arguments);

    // Wait task is done
    bool done = processCompile.waitForFinish(30000);

    const Core::String message = processCompile.readStdOutput() + u8" " + processCompile.readStdError();
    
    // Check timeout
    if (!done)
    {
        BT_THROW_ERROR_1(u8"Vulkan", u8"ShaderCompilationError", message);
    }
    // Check rc code
    if (processCompile.getReturnCode() != 0)
    {
        BT_THROW_ERROR_1(u8"Vulkan", u8"ShaderCompilationError", message);
    }
}

}
