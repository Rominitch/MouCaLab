/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    enum class Type : uint8_t
    {
        Unknown,
        UnsignedInt,
        Int,
        UnsignedChar,
        Char,
        UnsignedShort,
        Short,
        Float,
        Double,
        UnsignedInt64,
        Int64,
        NbTypes,

        // Alias
        UInt8  = UnsignedChar,
        Int8   = Char,
        UInt16 = UnsignedShort,
        Int16  = Short,
        UInt32 = UnsignedInt,
        Int32  = Int,
        UInt64 = UnsignedInt64        
    };

    constexpr size_t computeSizeOf(const Type eType)
    {
        if (eType == Type::Unknown || eType >= RT::Type::NbTypes)
            BT_THROW_ERROR(u8"BasicError", u8"UnknownEnumError");

        const std::array<size_t, static_cast<size_t>(RT::Type::NbTypes)> typeSizeOf
        { {
            0,
            sizeof(unsigned int),
            sizeof(int),
            sizeof(unsigned char),
            sizeof(char),
            sizeof(unsigned short),
            sizeof(short),
            sizeof(float),
            sizeof(double),
            sizeof(unsigned __int64),
            sizeof(__int64)
        } };
        return typeSizeOf[static_cast<size_t>(eType)];
    }

    // Default ID for usage
    enum class ComponentUsage : uint8_t
    {
        Anonyme,
        Vertex,
        Normal,
        Color,
        Tangent,
        BiTangent,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        TexCoord4,
        TexCoord5,
        TexCoord6,
        TexCoord7,
        Padding,
        BonesWeights,
        BonesIds,
        Index,
        Position,
        NbUsages
    };

    using GeoFloat = float;
    using Matrix2  = glm::mat2;
    using Matrix3  = glm::mat3;
    using Matrix4  = glm::mat4;
    using Point2i  = glm::ivec2;
    using Array2i  = glm::ivec2;
    using Vector2i = glm::ivec2;
    using Vector3i = glm::ivec3;
    using Vector4i = glm::ivec4;
    using Point2   = glm::vec2;
    using Point3   = glm::vec3;
    using Point4   = glm::vec4;
    using Vector2  = glm::vec2;
    using Vector3  = glm::vec3;
    using Vector4  = glm::vec4;
    using Array2   = glm::vec2;
    using Array3   = glm::vec3;
    using Array4   = glm::vec4;
    using Array2ui = glm::uvec2;
    using Array3ui = glm::uvec3;
    using Array4ui = glm::uvec4;

    using Vector2us = glm::lowp_uvec2;
    using Vector2d = glm::highp_f64vec2;

    static const double g_SQRT_2 = 1.4142135623730950488016887242097;

    struct VBOElement
    {
        GeoFloat m_Point[3];
        GeoFloat m_Normal[3];
        GeoFloat m_Texture[2];
    };

    //----------------------------------------------------------------------------
    /// \brief Generic IBO for triangle.
    struct IBOElement
    {
        IBOElement(const uint32_t index0, const uint32_t index1, const uint32_t index2):
        _index0(index0), _index1(index1), _index2(index2)
        {}
        //std::array<uint32_t, 3> _triangle;

        uint32_t _index0;
        uint32_t _index1;
        uint32_t _index2;
    };

    struct Edge
    {
        uint32_t _pts0;
        uint32_t _pts1;

        Edge() :
        _pts0( 0 ), _pts1( 0 )
        {}

        Edge( uint32_t pts0, uint32_t pts1 ) :
        _pts0( pts0 ), _pts1( pts1 )
        {}
    };

    struct SMouseState
    {
        Point2  _mousePositionSN;
        Point2  _lastMousePositionSN;
        Point2  _pickMousePositionSN;

        SMouseState() = default;

        void update()
        {
            _lastMousePositionSN = _mousePositionSN;
        }
    };

    /// Representing face order
    enum class FaceOrder : uint8_t
    {
        CounterClockWise,
        ClockWise,
        NbFaceOrder
    };
    
    /// Cull triangle
    enum class Culling : uint8_t
    {
        None,
        Front,
        Back,
        NbCulling
    };

    /// Kind of primitive to draw
    enum class Topology : uint8_t
    {
        Points,
        Lines,
        Triangles,
        Patches,

        NbTopology
    };

    /// Kind of shader
    enum class ShaderKind : uint8_t
    {
        Vertex,
        Fragment,
        Geometry,
        TessellationControl,
        TessellationEvaluation,

        NbShaders
    };

    struct ApplicationInfo
    {
        Core::String  _applicationName;
        uint32_t  _applicationVersion;
        Core::String  _engineName;
        uint32_t  _engineVersion;
    };
};