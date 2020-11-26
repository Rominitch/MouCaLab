/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
    vec4 gl_Position;
};

struct GlyphInfo
{
    vec4 bbox;

    // point offset
    // cell offset
    // cell count in x
    // cell count in y
    uvec4 cell_info;
};

layout (set = 0, binding = 0) buffer GlyphBuffer
{
	GlyphInfo glyphs[];
} glyph_buffer;

layout (set = 0, binding = 3) uniform Camera
{
    mat4 projection;
    mat4 view;
} camera;

layout(location = 0) in vec4 in_rect;
layout(location = 1) in uint in_glyph_index;
layout(location = 2) in float in_sharpness;

layout(location = 0) out vec2 out_glyph_pos;
layout(location = 1) out uvec4 out_cell_info;
layout(location = 2) out float out_sharpness;
layout(location = 3) out vec2 out_cell_coord;

void main()
{
    GlyphInfo gi = glyph_buffer.glyphs[in_glyph_index];

    vec4 pos[4];
	pos[0] = camera.projection * camera.view * vec4(in_rect.x, in_rect.y, 0.0, 1.0);
	pos[1] = camera.projection * camera.view * vec4(in_rect.z, in_rect.y, 0.0, 1.0);
	pos[2] = camera.projection * camera.view * vec4(in_rect.x, in_rect.w, 0.0, 1.0);
	pos[3] = camera.projection * camera.view * vec4(in_rect.z, in_rect.w, 0.0, 1.0);
    //pos[0] = vec4(in_rect.x, in_rect.y, 0.0, 1.0);
    //pos[1] = vec4(in_rect.z, in_rect.y, 0.0, 1.0);
    //pos[2] = vec4(in_rect.x, in_rect.w, 0.0, 1.0);
    //pos[3] = vec4(in_rect.z, in_rect.w, 0.0, 1.0);

    vec2 glyph_pos[4];
    glyph_pos[0] = vec2(gi.bbox.x, gi.bbox.y);
    glyph_pos[1] = vec2(gi.bbox.z, gi.bbox.y);
    glyph_pos[2] = vec2(gi.bbox.x, gi.bbox.w);
    glyph_pos[3] = vec2(gi.bbox.z, gi.bbox.w);

    vec2 cell_coord[4];
    cell_coord[0] = vec2(0,              0);
    cell_coord[1] = vec2(gi.cell_info.z, 0);
    cell_coord[2] = vec2(0,              gi.cell_info.w);
    cell_coord[3] = vec2(gi.cell_info.z, gi.cell_info.w);


    gl_Position = pos[gl_VertexIndex];
    out_glyph_pos = glyph_pos[gl_VertexIndex];
    out_cell_info = gi.cell_info;
    out_sharpness = in_sharpness;
    out_cell_coord = cell_coord[gl_VertexIndex];
}
