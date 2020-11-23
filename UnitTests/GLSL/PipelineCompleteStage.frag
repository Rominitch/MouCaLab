#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2DMS samplerPosition;
layout (binding = 2) uniform sampler2DMS samplerNormal;
layout (binding = 3) uniform sampler2DMS samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};

layout (binding = 4) uniform UBO 
{
    vec4 viewPos;
} ubo;

#define MAX_NUM_LIGHTS 100

layout (binding = 5) uniform Lighting 
{
    Light lights[MAX_NUM_LIGHTS];
    int   nbLights;
} lighting;

// Constant
layout (constant_id = 0) const int   NUM_SAMPLES = 8;
layout (constant_id = 1) const float AMBIENT     = 0.15;

// Manual resolve for MSAA samples 
vec4 resolve(sampler2DMS tex, ivec2 uv)
{
    vec4 result = vec4(0.0);	   
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        vec4 val = texelFetch(tex, uv, i); 
        result += val;
    }    
    // Average resolved samples
    return result / float(NUM_SAMPLES);
}

vec3 calculateLighting(vec3 pos, vec3 normal, vec4 albedo)
{
    vec3 result = vec3(0.0);

    for(int i = 0; i < lighting.nbLights; ++i)
    {
        // Vector to light
        vec3 L = lighting.lights[i].position.xyz - pos;
        // Distance from light to fragment position
        float dist = length(L);

        // Viewer to fragment
        vec3 V = ubo.viewPos.xyz - pos;
        V = normalize(V);
        
        // Light to fragment
        L = normalize(L);

        // Attenuation
        float atten = lighting.lights[i].radius / (pow(dist, 2.0) + 1.0);

        // Diffuse part
        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        if(NdotL > 0.0)
        {
            vec3 diff = lighting.lights[i].color * albedo.rgb * NdotL * atten;

            // Specular part
            vec3 R = reflect(-L, N);
            float NdotR = max(0.0, dot(R, V));
            vec3 spec = lighting.lights[i].color * albedo.a * pow(NdotR, 8.0) * atten;

            result += diff + spec;
        }
    }
    return result;
}

void main()
{
    vec2 orderUV = inUV;

    ivec2 attDim = textureSize(samplerPosition);
    ivec2 UV = ivec2(orderUV * attDim);

    // Ambient part
    vec4 alb = resolve(samplerAlbedo, UV);
    vec3 fragColor = vec3(0.0);
    
    // Calculate lighting for every MSAA sample
    for (int i = 0; i < NUM_SAMPLES; i++)
    { 
        vec3 pos = texelFetch(samplerPosition, UV, i).rgb;
        vec3 normal = texelFetch(samplerNormal, UV, i).rgb;
        vec4 albedo = texelFetch(samplerAlbedo, UV, i);
        fragColor += calculateLighting(pos, normal, albedo);
    }

    fragColor = (alb.rgb * AMBIENT) + fragColor / float(NUM_SAMPLES);
   
    outFragcolor = vec4(fragColor, 1.0);
}