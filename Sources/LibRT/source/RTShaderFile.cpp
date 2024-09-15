/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibCore/include/CoreStandardOS.h>

#include "LibRT/include/RTShaderFile.h"

namespace RT
{

ShaderFile::ShaderFile(const Core::Path& compiledPath, const ShaderKind kind, const Core::Path& filepath) :
Core::File(compiledPath, filepath.empty() ? Core::Resource::Mode::Critical : Core::Resource::Mode::AutoRefreshCritical), _kind(kind), _sourceFilepath(filepath)
{
    MouCa::preCondition(_sourceFilepath.empty() || std::filesystem::exists(_sourceFilepath) ); // DEV Issue: Need valid file if wanted.
    MouCa::preCondition(RT::ShaderKind::Vertex <= _kind && _kind < RT::ShaderKind::NbShaders);
    MouCa::postCondition(!isNull() && !isLoaded());
}

ShaderFile::~ShaderFile()
{
    MouCa::postCondition(!isLoaded());
}

void ShaderFile::compile()
{
    MouCa::preCondition(!_sourceFilepath.empty()); // DEV Issue: Need valid source file: API is consider like disabled.
    MouCa::preCondition(std::filesystem::exists(_sourceFilepath));
    MouCa::preCondition((std::filesystem::status(_sourceFilepath).permissions() & std::filesystem::perms::owner_read) != std::filesystem::perms::none);

    const Core::Path executable = Core::Path(Core::getEnvironmentVariable("VULKAN_SDK")) / u8"Bin" / u8"glslangValidator.exe";
    MouCa::preCondition(std::filesystem::exists(executable)); // DEV Issue: Not a valid path to glslangValidator, check VULKAN_SDK env.

    // Use basic command system: May be standard Process will be implemented soon (avoid boost::process to be light).
    const Core::String cmd = executable.string() + " -V -o " + _filename.string() + " " + _sourceFilepath.string();
    //const Core::String cmd = std::format("{} -V -o {} {}", executable, _filename, _sourceFilepath);
    const int rc = system(cmd.c_str());

    // Check rc code
    if (rc != 0)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "ShaderCompilationError") << std::format("Command: {}\nError code: {}", cmd, std::to_string(rc)));
    }
    else
    {
        MouCa::logConsole(std::format("GLSL compilation Success: {}", _sourceFilepath.filename().string()));
    }
}

}
