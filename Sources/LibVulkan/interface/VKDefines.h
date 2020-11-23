/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTBufferDescriptor.h>

namespace Vulkan
{
    //------------------------------------------------------------------------
    /// \brief  Basic descriptor for mesh (vertex/texCoord/color/normal/tangent).
    /// 
    /// \returns Descriptor for mesh.
    inline RT::BufferDescriptor getMeshDescriptor()
    {
        const std::vector<RT::ComponentDescriptor> descriptors =
        {
            { 3, RT::Type::Float, RT::ComponentUsage::Vertex },
            { 2, RT::Type::Float, RT::ComponentUsage::TexCoord0 },
            { 3, RT::Type::Float, RT::ComponentUsage::Color },
            { 3, RT::Type::Float, RT::ComponentUsage::Normal },
            { 3, RT::Type::Float, RT::ComponentUsage::Tangent }
        };
        RT::BufferDescriptor bufferDescriptor;
        bufferDescriptor.initialize( descriptors );
        return bufferDescriptor;
    }

    //------------------------------------------------------------------------
    /// \brief  Basic descriptor for mesh (vertex/texCoord/color/normal/tangent/bonesWeight/bonesId).
    /// 
    /// \returns Descriptor for mesh.
    inline RT::BufferDescriptor getMeshAnimationDescriptor()
    {
        const std::vector<RT::ComponentDescriptor> descriptors =
        {
            { 3, RT::Type::Float, RT::ComponentUsage::Vertex },
            { 2, RT::Type::Float, RT::ComponentUsage::TexCoord0 },
            { 3, RT::Type::Float, RT::ComponentUsage::Color },
            { 3, RT::Type::Float, RT::ComponentUsage::Normal },
            { 3, RT::Type::Float, RT::ComponentUsage::Tangent },
            { 4, RT::Type::Float, RT::ComponentUsage::BonesWeights },
            { 4, RT::Type::Int,   RT::ComponentUsage::BonesIds }
        };
        RT::BufferDescriptor bufferDescriptor;
        bufferDescriptor.initialize(descriptors);
        return bufferDescriptor;
    }
}