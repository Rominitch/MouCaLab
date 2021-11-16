/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKShaderProgram.h"

namespace Vulkan
{

const std::array<RT::ShaderKind, static_cast<size_t>(RT::ShaderKind::NbShaders)> ShaderProgram::_allShaderType =
{{
    RT::ShaderKind::Vertex,
    RT::ShaderKind::Fragment,
    RT::ShaderKind::Geometry,
    RT::ShaderKind::TessellationControl,
    RT::ShaderKind::TessellationEvaluation
}};

ShaderModule::ShaderModule() :
_shaderModule(VK_NULL_HANDLE), _stage(static_cast<VkShaderStageFlagBits>(0))
{
    MouCa::preCondition(isNull());
}

ShaderModule::~ShaderModule()
{
    MouCa::preCondition(isNull());
}

void ShaderModule::initialize(const Device& device, const Core::String& shaderSource, const std::string& name, const VkShaderStageFlagBits stage)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!shaderSource.empty());
    MouCa::preCondition(!name.empty());

    _name  = name;
    _stage = stage;

    //Build info
    const VkShaderModuleCreateInfo shaderModuleCreateInfo =
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,            // VkStructureType                sType
        nullptr,                                                // const void                    *pNext
        0,                                                      // VkShaderModuleCreateFlags      flags
        shaderSource.size(),                                    // size_t                         codeSize
        reinterpret_cast<const uint32_t*>(shaderSource.c_str()) // const uint32_t                *pCode
    };

    //Create shader
    if (vkCreateShaderModule(device.getInstance(), &shaderModuleCreateInfo, nullptr, &_shaderModule) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "ShaderCreationError"));
    }

    MouCa::postCondition(!isNull());
}

void ShaderModule::release(const Device& device)
{
    MouCa::assertion(!isNull());
    MouCa::assertion(!device.isNull());

    vkDestroyShaderModule(device.getInstance(), _shaderModule, nullptr);
    _shaderModule = VK_NULL_HANDLE;
}

ShaderProgram::ShaderProgram():
_shaderModule(VK_NULL_HANDLE)
{
    MouCa::preCondition(isNull());
}

ShaderProgram::~ShaderProgram()
{
    MouCa::preCondition(isNull());
}

void ShaderProgram::initialize(const Device& device, const Core::File& shaderSourceFile, const std::string& name)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(shaderSourceFile.isExist());
    MouCa::preCondition(!name.empty());

    _name = name;

    //Read file data
    const std::string code = shaderSourceFile.extractString();
    if(code.empty())
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "ShaderFileEmptyError") << shaderSourceFile.getFilePath().string());
    }

    //Build info
    VkShaderModuleCreateInfo shaderModuleCreateInfo =
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,    // VkStructureType                sType
        nullptr,                                        // const void                    *pNext
        0,                                              // VkShaderModuleCreateFlags      flags
        code.size(),                                    // size_t                         codeSize
        reinterpret_cast<const uint32_t*>(code.c_str()) // const uint32_t                *pCode
    };

    //Create shader
    if(vkCreateShaderModule(device.getInstance(), &shaderModuleCreateInfo, nullptr, &_shaderModule) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "ShaderCreationError") << shaderSourceFile.getFilePath().string());
    }
}

void ShaderProgram::release(const Device& device)
{
    MouCa::assertion(!isNull());
    MouCa::assertion(!device.isNull());
    
    vkDestroyShaderModule(device.getInstance(), _shaderModule, nullptr);
    _shaderModule = VK_NULL_HANDLE;
}

}
