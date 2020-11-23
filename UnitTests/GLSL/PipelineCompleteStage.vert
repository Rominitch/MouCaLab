#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
    mat4 view;
} ubo;

//#define PHYSICAL_SUPPORT

#ifdef PHYSICAL_SUPPORT
    layout (location = 0) in vec2 inVer;
    layout (location = 1) in vec2 inUV;
#endif

layout (location = 0) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
#ifdef PHYSICAL_SUPPORT
    outUV = inUV;
    gl_Position = ubo.projection * vec4(inVer, 0.0f, 1.0f);
#else
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
#endif
}
