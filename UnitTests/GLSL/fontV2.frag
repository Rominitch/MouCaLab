#version 450
#extension GL_ARB_separate_shader_objects : enable

#define UDIST_BIAS 0.001

struct Points
{
    vec2  pts;
    uint  code;
};

struct GlyphInfo
{
    vec4  bbox;
    uvec2 index;
};



layout (set = 0, binding = 0) buffer GlyphBuffer
{
    GlyphInfo glyphs[];
} glyph_buffer;

layout (set = 0, binding = 1) buffer PointBuffer
{
    Points points[];
} pointBuffer;

layout (set = 0, binding = 2) buffer ControlBuffer
{
    vec2 points[];
} controlBuffer;

layout(location = 0) in vec2       in_glyph_pos;
layout(location = 1) flat in uvec2 in_IndexPts;
layout(location = 2) in vec2       in_TexCoord;
layout(location = 3) flat in uint  in_glyph_index;

layout(location = 0) out vec4 out_color;

const uint  shiftCode         = 3;
const float kQuadraticEpsilon = 0.0001;
//const float maxFloat          = intBitsToFloat(2139095039);

float calc_t(vec2 a, vec2 b, vec2 p)
{
    vec2 dir = b - a;
    return dot(p - a, dir) / dot(dir, dir);
}

float dist_to_line(vec2 a, vec2 b, vec2 p)
{
    vec2 dir = b - a;
    vec2 norm = vec2(-dir.y, dir.x);
    return dot(normalize(norm), a - p);
}

float dist_to_bezier2(vec2 p0, vec2 p1, vec2 p2, float t, vec2 p)
{
    vec2 q0 = mix(p0, p1, t);
    vec2 q1 = mix(p1, p2, t);
    return dist_to_line(q0, q1, p);
}

float dist_to_bezier3(vec2 p0, vec2 c0, vec2 c1, vec2 p1, float t, vec2 p)
{
    vec2 q0 = mix(p0, c0, t);
    vec2 q1 = mix(c0, c1, t);
    vec2 q2 = mix(c1, p1, t);
    vec2 r0 = mix(q0, q1, t);
    vec2 r1 = mix(q1, q2, t);
    return dist_to_line(r0, r1, p);
}

float linearX(in vec2 p1, in vec2 p2, in vec2 pixelsPerEm)
{
    float coverage = 0.0;
    
    if(max(p1.x, p2.x) * pixelsPerEm.x < -0.5)
        return coverage;

    bool way = p1.y > 0.0;
    if (way ^^ (p2.y > 0.0))
    {
        vec2 p2_1 = p2 - p1;
        
        float i;
        
        if(abs(p2_1.y) < kQuadraticEpsilon)
            i = p1.x;
        else
        {
            float t = -p1.y / p2_1.y;
            i = p1.x + t * p2_1.x;
        }
        i = clamp(i * pixelsPerEm.x + 0.5, 0.0, 1.0);
        
        if (way) coverage += i;
        else     coverage -= i;
    }
    return coverage;
}

float linearY(in vec2 p1, in vec2 p2, in vec2 pixelsPerEm)
{
    float coverage = 0.0;
    
    if(max(p1.y, p2.y) * pixelsPerEm.y < -0.5)
        return coverage;
    
    bool way = p1.x > 0.0;
    if (way ^^ (p2.x > 0.0))
    {
        vec2 p2_1 = p2 - p1;
        
        float i;
        
        if(abs(p2_1.x) < kQuadraticEpsilon)
            i = p1.y;
        else
        {
            float t = -p1.x / p2_1.x;
            i = p1.y + t * p2_1.y;
        }
        i = clamp(i * pixelsPerEm.y + 0.5, 0.0, 1.0);
        
        if (way) coverage -= i;
        else     coverage += i;
    }
    return coverage;
}

float bezier2CoverageX(in vec2 p1, in vec2 p2, in vec2 p3, in vec2 pixelsPerEm)
{
    float coverage = 0.0;
    if (max(max(p1.x, p2.x), p3.x) * pixelsPerEm.x < -0.5)
        return 0.0;

    // Generate the root contribution code based on the signs of the
    // y coordinates of the three control points.

    uint code = (0x2E74U >> (((p1.y > 0.0) ? 2U : 0U) +
                ((p2.y > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;

    if (code != 0U)
    {
        // At least one root makes a contribution, so solve for the
        // values of t where the curve crosses y = 0. The quadratic
        // polynomial in t is given by
        //
        //     a t^2 - 2b t + c,
        //
        // where a = p1.y - 2 p2.y + p3.y, b = p1.y - p2.y, and c = p1.y.
        // The discriminant b^2 - ac is clamped to zero, and imaginary
        // roots are treated as a double root at the global minimum
        // where t = b / a.

        vec2  a = p1 - p2 * 2.0 + p3;
        vec2  b = p1 - p2;
        float ra = 1.0 / a.y;

        float t1;
        float t2;

        // If the polynomial is nearly linear, then solve -2b t + c = 0.
        if (abs(a.y) < kQuadraticEpsilon)
        {
            t1 = t2 = p1.y * 0.5 / b.y;
        }
        else
        {
            float d = sqrt(max(b.y * b.y - a.y * p1.y, 0.0));
            t1 = (b.y - d) * ra;
            t2 = (b.y + d) * ra;
        }

        // Calculate the x coordinates where C(t) = 0, and transform
        // them so that the current pixel corresponds to the range
        // [0,1]. Clamp the results and use them for root contributions.
        float x1 = (a.x * t1 - b.x * 2.0) * t1 + p1.x;
        float x2 = (a.x * t2 - b.x * 2.0) * t2 + p1.x;
        x1 = clamp(x1 * pixelsPerEm.x + 0.5, 0.0, 1.0);
        x2 = clamp(x2 * pixelsPerEm.x + 0.5, 0.0, 1.0);

        // Bits in code tell which roots make a contribution.
        if ((code & 1U) != 0U) coverage += x1;
        if (code > 1U)         coverage -= x2;
    }
    
    return coverage;
}

float bezier2CoverageY(in vec2 p1, in vec2 p2, in vec2 p3, in vec2 pixelsPerEm)
{
    float coverage = 0.0;
    if (max(max(p1.y, p2.y), p3.y) * pixelsPerEm.y < -0.5)
        return 0.0;
    
    uint code = (0x2E74U >> (((p1.x > 0.0) ? 2U : 0U) +
                ((p2.x > 0.0) ? 4U : 0U) + ((p3.x > 0.0) ? 8U : 0U))) & 3U;

    if (code != 0U)
    {
        // At least one root makes a contribution, so solve for the
        // values of t where the rotated curve crosses y = 0.
        vec2  a = p1 - p2 * 2.0 + p3;
        vec2  b = p1 - p2;
        float ra = 1.0 / a.x;

        float t1;
        float t2;

        if (abs(a.x) < kQuadraticEpsilon)
            t1 = t2 = p1.x * 0.5 / b.x;
        else
        {
            float d = sqrt(max(b.x * b.x - a.x * p1.x, 0.0));
            t1 = (b.x - d) * ra;
            t2 = (b.x + d) * ra;
        }

        float x1 = (a.y * t1 - b.y * 2.0) * t1 + p1.y;
        float x2 = (a.y * t2 - b.y * 2.0) * t2 + p1.y;
        x1 = clamp(x1 * pixelsPerEm.y + 0.5, 0.0, 1.0);
        x2 = clamp(x2 * pixelsPerEm.y + 0.5, 0.0, 1.0);

        if ((code & 1U) != 0U) coverage -= x1;
        if (code > 1U)         coverage += x2;
    }
    return coverage;
}

float linearCoverage(in vec2 p1, in vec2 p2, in vec2 pixelsPerEm)
{
    return linearX(p1, p2, pixelsPerEm);
         + linearY(p1, p2, pixelsPerEm);
}

float bezier2Coverage(in vec2 p1, in vec2 p2, in vec2 p3, in vec2 pixelsPerEm)
{
    return bezier2CoverageX(p1, p2, p3, pixelsPerEm);
         + bezier2CoverageY(p1, p2, p3, pixelsPerEm);
}

void showPoints()
{
    vec4 color[] = 
    {
        vec4(0.0, 0.5, 0.5, 1.0),
        vec4(0.0, 0.4, 0.7, 1.0),
        vec4(0.0, 0.7, 0.4, 1.0)
    };
    
    for (uint i = in_IndexPts.x; i < in_IndexPts.y; i += 1)
    {
        uint code    = pointBuffer.points[i].code & 0x3;
        uint indexPC = pointBuffer.points[i].code >> shiftCode;
    
        float d = distance(in_TexCoord, pointBuffer.points[i].pts);
        if( d < 0.01f )
        {
            if( (pointBuffer.points[i].code & 0x4) == 0x4 )
                out_color = vec4(0.1,0.1,0.9,1);
            else
                out_color = color[code];
            return;
        }

        for(uint pc = 0; pc < code; pc += 1)
        {
            float d = distance(in_TexCoord, controlBuffer.points[indexPC + pc]);
            if( d < 0.01f )
            {
                out_color = vec4(0.6,0,0.6,1);
                return;
            }
        }
    }
}

vec2 nextLoop(uint i)
{
    if( (pointBuffer.points[i+1].code & 0x4) == 0x4 )
    {
        uint indexPC = pointBuffer.points[i].code >> shiftCode;
        return pointBuffer.points[indexPC].pts;
    }
    return pointBuffer.points[i+1].pts;
}

void main()
{
    vec2 pixelsPerEm = vec2(1.0 / fwidth(in_TexCoord.x),
                            1.0 / fwidth(in_TexCoord.y));
    
    float coverage = 0.0;

    out_color = vec4(0, 0, 0, 0);
    //uint aa = in_IndexPts.x+0;
    //for (uint i = aa; i < min(in_IndexPts.y-1, aa+3); i += 1)
    for (uint i = in_IndexPts.x; i < in_IndexPts.y-1; i += 1)
    {
        // Search outline loop
        if( (pointBuffer.points[i+1].code & 0x4) == 0x4 )
        {
            continue;
        }
        
        // First and last control point
        vec2 s    = pointBuffer.points[i].pts;
        vec2 e    = pointBuffer.points[i+1].pts;
        
        // Get type of line
        uint type = pointBuffer.points[i+1].code & 0x3;
        float currentDist;
        if( type == 0 ) // Linear
        {
            vec2 p1 = s - in_TexCoord;
            vec2 p2 = e - in_TexCoord;
            coverage += linearCoverage(p1, p2, pixelsPerEm);
        }
        else if( type == 1 ) // Bezier quadratic
        {
            uint indexPC = pointBuffer.points[i+1].code >> shiftCode;
            
                
            vec2 p1 = s - in_TexCoord;
            vec2 p2 = controlBuffer.points[indexPC] - in_TexCoord;
            vec2 p3 = e - in_TexCoord;
                        
            coverage += bezier2Coverage(p1, p2, p3, pixelsPerEm);
        }
    }
    
    out_color = vec4(0, 0, 0, coverage);
    
    //showPoints();
}
