/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

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
    MOUCA_PRE_CONDITION(isNull());
}

ShaderModule::~ShaderModule()
{
    MOUCA_PRE_CONDITION(isNull());
}

void ShaderModule::initialize(const Device& device, const Core::String& shaderSource, const std::string& name, const VkShaderStageFlagBits stage)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(!shaderSource.empty());
    MOUCA_PRE_CONDITION(!name.empty());

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
        BT_THROW_ERROR(u8"Vulkan", u8"ShaderCreationError");
    }

    MOUCA_POST_CONDITION(!isNull());
}

void ShaderModule::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());
    MOUCA_ASSERT(!device.isNull());

    vkDestroyShaderModule(device.getInstance(), _shaderModule, nullptr);
    _shaderModule = VK_NULL_HANDLE;
}

ShaderProgram::ShaderProgram():
_shaderModule(VK_NULL_HANDLE)
{
    MOUCA_PRE_CONDITION(isNull());
}

ShaderProgram::~ShaderProgram()
{
    MOUCA_PRE_CONDITION(isNull());
}

void ShaderProgram::initialize(const Device& device, const Core::File& shaderSourceFile, const std::string& name)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(shaderSourceFile.isExist());
    MOUCA_PRE_CONDITION(!name.empty());

    _name = name;

    //Read file data
    const std::string code = shaderSourceFile.extractString();
    if(code.empty())
    {
        BT_THROW_ERROR_1(u8"Vulkan", u8"ShaderFileEmptyError", Core::convertToU8(shaderSourceFile.getFilePath()));
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
        BT_THROW_ERROR_1(u8"Vulkan", u8"ShaderCreationError", Core::convertToU8(shaderSourceFile.getFilePath()));
    }
}

void ShaderProgram::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());
    MOUCA_ASSERT(!device.isNull());
    
    vkDestroyShaderModule(device.getInstance(), _shaderModule, nullptr);
    _shaderModule = VK_NULL_HANDLE;
}

}
