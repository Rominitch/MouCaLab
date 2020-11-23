#version 450

layout (binding = 0) uniform sampler2DMS samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

layout (constant_id = 0) const int   NUM_SAMPLES = 8;

void main() 
{
    ivec2 attDim = textureSize(samplerAlbedo);
    ivec2 UV     = ivec2(inUV * attDim);
    vec3 fragColor = vec3(0.0);
    
    // Calculate color for every MSAA sample
    for (int i = 0; i < NUM_SAMPLES; i++)
    { 
        fragColor += texelFetch(samplerAlbedo, UV, i).rgb;
    }
    // Compute mean
    fragColor = fragColor / float(NUM_SAMPLES);
    
    outFragcolor = vec4(fragColor, 1.0);
}