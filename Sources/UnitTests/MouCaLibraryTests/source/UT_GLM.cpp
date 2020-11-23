#include "Dependancies.h"

///------------------------------------------------------------------------------------------------
namespace testing
{

/// GLM for GoogleTest
inline AssertionResult AssertVec3Helper( const char* e1,
                                  const char* e2,
                                  const char* prec,
                                  const glm::vec3& v1,
                                  const glm::vec3& v2,
                                  const float precision )
{
    if( fabs( v1.x-v2.x ) < precision
     && fabs( v1.y-v2.y ) < precision
     && fabs( v1.z-v2.z ) < precision )
        return AssertionSuccess();

    return AssertionFailure() << "error: The difference between("
        << e1 << ", "
        << e2 << "), where"
        << "\n" << e1 << " evaluates to " << "glm::vec3( " << v1.x << ", " << v1.y << ", " << v1.z << " )"
        << "\n" << e2 << " evaluates to " << "glm::vec3( " << v2.x << ", " << v2.y << ", " << v2.z << " )"
        << "\n which exceeds " << prec << " evaluates to " << precision;
}

inline AssertionResult AssertVec4Helper( const char* e1,
                                  const char* e2,
                                  const char* prec,
                                  const glm::vec4& v1,
                                  const glm::vec4& v2,
                                  const float precision )
{
    if( fabs( v1.x-v2.x ) < precision
     && fabs( v1.y-v2.y ) < precision
     && fabs( v1.z-v2.z ) < precision
     && fabs( v1.w-v2.w ) < precision)
        return AssertionSuccess();

    return AssertionFailure() << "error: The difference between("
        << e1 << ", "
        << e2 << "), where"
        << "\n" << e1 << " evaluates to " << "glm::vec4( " << v1.x << ", " << v1.y << ", " << v1.z << ", " << v1.w << " )"
        << "\n" << e2 << " evaluates to " << "glm::vec4( " << v2.x << ", " << v2.y << ", " << v2.z << ", " << v2.w << " )"
        << "\n which exceeds " << prec << " evaluates to " << precision;
}

inline AssertionResult AssertMat4Helper( const char* e1,
                                  const char* e2,
                                  const char* prec,
                                  const glm::mat4& m1,
                                  const glm::mat4& m2,
                                  const float precision )
{
    bool valid = true;
    for( int rawID=0; rawID < 4; ++rawID )
    {
        for( int columnID=0; columnID < 4; ++columnID )
        {
            valid &= fabs( m1[rawID][columnID]-m2[rawID][columnID] ) < precision;
        }
    }

    if( valid )
        return AssertionSuccess();

    return AssertionFailure() << "error: The difference between("
        << e1 << ", "
        << e2 << "), where"
        << "\n" << e1 << " evaluates to\n" //<< print( v1 ).c_str()
        << "\n" << e2 << " evaluates to\n" //<< print( v2 ).c_str()
        << "\n which exceeds " << prec << " evaluates to " << precision;
}

#ifndef EXPECT_VEC3_NEAR
#define EXPECT_VEC3_NEAR(vectorRef, vectorCmp, precision) \
            GTEST_ASSERT_(testing::AssertVec3Helper(#vectorRef, #vectorCmp, #precision, vectorRef, vectorCmp, precision), GTEST_NONFATAL_FAILURE_)
#endif

#ifndef EXPECT_VEC4_NEAR
#define EXPECT_VEC4_NEAR(vectorRef, vectorCmp, precision) \
            GTEST_ASSERT_(testing::AssertVec4Helper(#vectorRef, #vectorCmp, #precision, vectorRef, vectorCmp, precision), GTEST_NONFATAL_FAILURE_)
#endif

#ifndef EXPECT_MAT4_NEAR
#define EXPECT_MAT4_NEAR(matrixRef, matrixCmp, precision) \
            GTEST_ASSERT_(testing::AssertMat4Helper(#matrixRef, #matrixCmp, #precision, matrixRef, matrixCmp, precision), GTEST_NONFATAL_FAILURE_)
#endif
}
///------------------------------------------------------------------------------------------------

/// Default constante
const float precision = 1e-6f;
const float PI        = 3.14159265358979323846f;
const float sqrt_2    = sqrt( 2.0f );

///------------------------------------------------------------------------------------------------

TEST( GLM, GLM )
{
    glm::vec4 b( 3.0f, 2.0f, 0.0f, 1.0f );

    glm::vec4 a( 1.0f, 1.0f, 0.0f, 1.0f );
    glm::mat4 t( 1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    2.0f, 1.0f, 0.0f, 1.0f );
    EXPECT_EQ( b, t * a );

    glm::mat4 t2( 1.0f ); // Identity
    t2 = glm::translate( t2, glm::vec3( 2.0f, 1.0f, 0.0f ) );
    EXPECT_EQ( b, t2 * a );
}

TEST( GLM, GLM2 )
{
    glm::mat4 m( 0.0f, 1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 3.0f, 1.0f, 0.0f, 1.0f );

    glm::mat4 op = glm::translate( glm::mat4( 1.0f ), glm::vec3( 1.0f, 2.0f, 0.0f ) );
    glm::mat4 po = glm::inverse( op );
    glm::mat4 r  = glm::rotate( glm::mat4( 1.0f ), PI*0.5f, glm::vec3( 0.0f, 0.0f, 1.0f ) );

    glm::mat4 c = op * r * po;

    // Near expected
    EXPECT_MAT4_NEAR( m, c, precision );
}

TEST( GLM, GLM3 )
{
    glm::mat4 m( 0.0f, 1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 3.0f, 1.0f, 0.0f, 1.0f );

    glm::vec4 a( 1.0f, 2.0f, 0.0f, 1.0f );
    glm::vec4 b( 4.0f/3.0f, 4.0f/3.0f, 0.0f, 1.0f );
    glm::vec4 c( 1.0f, 2.0f, 0.0f, 1.0f );
    glm::vec4 d( 5.0f/3.0f, 7.0f/3.0f, 0.0f, 1.0f );

    EXPECT_EQ( c, m * a );

    glm::vec4 computed = m * b;
    // Near expected
    EXPECT_VEC4_NEAR( d, computed, precision );
}

TEST(GLM, mvp)
{
    glm::mat4 m(0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 v(0.0f, 1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 p(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                1.0f, 2.0f, 3.0f, 1.0f);

    glm::mat4 result = p * v * m;
    glm::mat4 mvp(0.0f, 0.5f, 0.0f, 0.0f,
                  0.5f, 0.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 0.5f, 0.0f,
                  1.0f, 2.0f, 3.0f, 1.0f);
    EXPECT_EQ(mvp, result);
    EXPECT_NE(mvp, m * v * p);
}

TEST(GLM, rotate)
{
    const float pi_4 = 0.785398163397448309616f;
    const float C = 1.0f / sqrt(2.0f);
    glm::mat4 r(C, C, 0.0f, 0.0f,
                -C, C, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 r1(1.0f); // Identity
    r1 = glm::rotate(r1, pi_4, glm::vec3(0.0f, 0.0f, 1.0f));

    EXPECT_EQ(r, r1);
}

TEST(GLM, rotate2)
{
    const float C = 1.0f / sqrt(2.0f);
    glm::mat4 r(C, C, 0.0f, 0.0f,
                -C, C, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

    glm::vec4 a(2.0f/3.0f, 1.0f/3.0f, 0.0f, 1.0f);
    glm::vec4 b(C/3.0f, C, 0.0f, 1.0f);

    EXPECT_EQ(b, r * a);
}

TEST(GLM, scaling)
{
    glm::vec3 h(0.5f, 0.5f, 0.5f);
    glm::mat4 m(h.x, 0.0f, 0.0f, 0.0f,
                0.0f, h.y, 0.0f, 0.0f,
                0.0f, 0.0f, h.z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 m1(1.0f); // Identity
    m1 = glm::scale(m1, h);
    EXPECT_EQ(m1, m);

    glm::vec4 a(2.0f/3.0f, 1.0f/3.0f, 0.0f, 1.0f);
    glm::vec4 b(1.0f, 1.0f/3.0f, 0.0f, 1.0f);

    glm::vec4 c(1.0f/3.0f, 1.0f/6.0f, 0.0f, 1.0f);
    glm::vec4 d(1.0f/2.0f, 1.0f/6.0f, 0.0f, 1.0f);

    EXPECT_EQ(c, m * a);
    EXPECT_EQ(d, m * b);
}

TEST(GLM, composed)
{
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), PI*0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(11.0f/6.0f, 0.5f, 0.0f));

    glm::mat4 m(0.0f, 0.5f, 0.0f, 0.0f,
                -0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                11.0f/6.0f, 0.5f, 0.0f, 1.0f);
    glm::mat4 c = t * r * s;

    // Near except
    EXPECT_MAT4_NEAR( m, c, precision );
}

TEST(GLM, mulComposed)
{
    glm::mat4 m(0.0f, 0.5f, 0.0f, 0.0f,
                -0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                11.0f/6.0f, 0.5f, 0.0f, 1.0f);
    glm::vec4 a(2.0f/3.0f, 1.0f/3.0f, 0.0f, 1.0f);
    glm::vec4 b(5.0f/3.0f, 5.0f/6.0f, 0.0f, 1.0f);

    glm::vec4 mul = m * a;

    // Near except
    EXPECT_VEC4_NEAR( b, mul, precision );
}


struct Orientation
{
    glm::quat _rotation;
    glm::vec3 _position;
    glm::vec3 _homothetie;

    Orientation():
    _homothetie(1.0f, 1.0f, 1.0f)
    {}

    Orientation(const Orientation& copy) :
    _rotation( copy._rotation ),
    _position( copy._position ),
    _homothetie( copy._homothetie )
    {}

    void inverse()
    {
        _homothetie  = 1.0f / _homothetie;
        _rotation = glm::inverse( _rotation );
        _position = (_rotation * (_homothetie * -_position));
    }

    // World = This * Local
    Orientation operator * ( const Orientation& localSpace ) const
    {
        Orientation worldSpace;
        worldSpace._position = _position + _rotation * (localSpace._position * _homothetie);
        worldSpace._rotation = _rotation * localSpace._rotation;
        worldSpace._homothetie  = _homothetie * localSpace._homothetie;
        return worldSpace;
    }

    glm::vec3 operator * (const glm::vec3& localPoint) const
    {
#ifdef _DEBUG
        glm::vec3 s = (localPoint * _homothetie);
        glm::vec3 r = _rotation * s;

        return _position + r;
#else
        return _position + _rotation * (localPoint * _homothetie);
#endif
    }

    static Orientation lerp(const Orientation& a, const Orientation& b, const float factor)
    {
        Orientation interpole;
        interpole._homothetie = a._homothetie + (b._homothetie - a._homothetie) * factor;
        interpole._rotation   = glm::lerp( a._rotation, b._rotation, factor );
        interpole._position   = a._position + (b._position - a._position) * factor;
        return interpole;
    }

    static Orientation slerp( const Orientation& a, const Orientation& b, const float factor )
    {
        Orientation interpole;
        interpole._homothetie = a._homothetie + (b._homothetie - a._homothetie) * factor;
        interpole._rotation   = glm::slerp( a._rotation, b._rotation, factor );
        interpole._position   = a._position + (b._position - a._position) * factor;
        return interpole;
    }
};

TEST( GLM, quaternion_mul )
{
    const float angle = PI * 0.5f;
    glm::vec3 om(11.0f/6.0f, 0.5f, 0.0f );
    glm::vec3 scale( 0.5f, 0.5f, 0.5f );

    glm::vec4 a( 4.0f/6.0f, 2.0f/6.0f, 0.0f, 1.0f );
    glm::vec4 c( 10.0f/6.0f, 5.0f/6.0f, 0.0f, 1.0f );

    glm::mat4 mc;
    {
        glm::mat4 s = glm::scale( glm::mat4( 1.0f ), scale );
        glm::mat4 r = glm::rotate( glm::mat4( 1.0f ), angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );
        glm::mat4 t = glm::translate( glm::mat4( 1.0f ), om );

        mc = t * r * s;

        // Near except
        glm::vec4 cal_c = mc * a;
        EXPECT_VEC4_NEAR( c, cal_c, 1e-6f );
    }

    Orientation qc;
    {
        qc._position = om;
        qc._homothetie  = scale;
        qc._rotation = glm::angleAxis( angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );

        // Near except
        glm::vec3 cal_c = qc * a;
        EXPECT_VEC3_NEAR( c, cal_c, precision );
    }
}

TEST( GLM, quaternion_mul_non_uniform )
{
    const float angle = PI * 0.5f;
    glm::vec3 om( 11.0f/6.0f, 0.5f, 0.0f );
    glm::vec3 scale( 0.5f, 1.0f, 0.5f );

    glm::vec4 a( 4.0f/6.0f, 2.0f/6.0f, 0.0f, 1.0f );
    glm::vec4 c( 9.0f/6.0f, 5.0f/6.0f, 0.0f, 1.0f );

    glm::mat4 mc;
    {
        glm::mat4 s = glm::scale( glm::mat4( 1.0f ), scale );
        glm::mat4 r = glm::rotate( glm::mat4( 1.0f ), angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );
        glm::mat4 t = glm::translate( glm::mat4( 1.0f ), om );

        mc = t * r * s;

        // Near except
        glm::vec4 cal_c = mc * a;
        EXPECT_VEC4_NEAR( c, cal_c, precision );
    }

    Orientation qc;
    {
        qc._position = om;
        qc._homothetie  = scale;
        qc._rotation = glm::angleAxis( angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );

        // Near except
        glm::vec3 cal_c = qc * a;
        EXPECT_VEC3_NEAR( c, cal_c, precision );
    }
}

TEST( GLM, quaternion_mul_non_uniform_PI_4 )
{
    const float angle = PI * 0.25f;
    glm::vec3 om( 11.0f/6.0f, 0.5f, 0.0f );
    glm::vec3 scale( 0.5f, 1.0f, 0.5f );

    glm::vec4 a( 4.0f/6.0f, 2.0f/6.0f, 0.0f, 1.0f );
    glm::vec4 c( 1.83333337f, 0.971404552f, 0.0f, 1.0f );
    glm::mat4 mc;
    {
        glm::mat4 s = glm::scale( glm::mat4( 1.0f ), scale );
        glm::mat4 r = glm::rotate( glm::mat4( 1.0f ), angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );
        glm::mat4 t = glm::translate( glm::mat4( 1.0f ), om );

        mc = t * r * s;

        // Near except
        glm::vec4 cal_c = mc * a;
        EXPECT_VEC4_NEAR( c, cal_c, precision );
    }

    Orientation qc;
    {
        qc._position = om;
        qc._homothetie  = scale;
        qc._rotation = glm::angleAxis( angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );

        // Near except
        glm::vec3 cal_c = qc * a;
        EXPECT_VEC3_NEAR( c, cal_c, precision );
    }
}

TEST( GLM, quaternion_multiplication_way )
{
    const float angle = PI * 0.5f;
    glm::quat q = glm::angleAxis( angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );

    glm::vec4 a(  1.0f, 1.0f, 0.0f, 1.0f );
    glm::vec4 c( -1.0f, 1.0f, 0.0f, 1.0f );

    glm::vec3 cal_c = q * a;
    EXPECT_VEC3_NEAR( c, cal_c, precision );

    glm::vec3 cal_a = c * q;
    EXPECT_VEC3_NEAR( a, cal_a, precision );

    cal_a = (q * a) * q;
    EXPECT_VEC3_NEAR( a, cal_a, precision );
    cal_a = q * (a * q);
    EXPECT_VEC3_NEAR( a, cal_a, precision );
}

TEST( GLM, quaternion_inverse )
{
    const float angle = PI * 0.5f;
    glm::vec3 om( 11.0f/6.0f, 0.5f, 0.0f );
    glm::vec3 scale( 0.5f, 0.5f, 0.5f );

    glm::vec4 a( 4.0f/6.0f, 2.0f/6.0f, 0.0f, 1.0f );
    glm::vec4 c( 10.0f/6.0f, 5.0f/6.0f, 0.0f, 1.0f );

    Orientation qc;
    qc._position = om;
    qc._homothetie  = scale;
    qc._rotation = glm::angleAxis( angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );

    Orientation qa(qc);
    qa.inverse();

    glm::vec3 cal_c = qc * a;
    EXPECT_VEC3_NEAR( c, cal_c, precision );

    glm::vec3 cal_a = qa * c;
    EXPECT_VEC3_NEAR( a, cal_a, precision );
}

TEST( GLM, quaternion )
{
    glm::vec3 oa( 1.0f, 2.0f, 0.0f );

    glm::vec4 b(4.0f/3.0f, 4.0f/3.0f, 0.0f, 1.0f);
    glm::vec4 d(5.0f/3.0f, 7.0f/3.0f, 0.0f, 1.0f);

    glm::mat4 mc;
    {
        glm::mat4 m( 0.0f, 1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     3.0f, 1.0f, 0.0f, 1.0f );

        glm::mat4 op = glm::translate( glm::mat4( 1.0f ), oa );
        glm::mat4 po = glm::inverse( op );
        glm::mat4 r  = glm::rotate( glm::mat4( 1.0f ), PI*0.5f, glm::vec3( 0.0f, 0.0f, 1.0f ) );

        mc = op * r * po;

        // Near except
        glm::vec4 cal_d = mc * b;
        EXPECT_VEC4_NEAR( d, cal_d, precision );
    }

    Orientation qc;
    {
        Orientation op;
        op._position = oa;
        Orientation po;
        po._position = -oa;
        Orientation r;
        r._rotation = glm::angleAxis(PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        
        qc = op * r * po;

        // Near except
        glm::vec3 cal_d = mc * b;
        EXPECT_VEC3_NEAR( d, cal_d, precision );
    }
}

TEST( GLM, quaternion_SRT )
{
    glm::vec3 homothetie( 0.5f, 0.5f, 0.5f );

    float angle = PI / 2.0f;
    glm::vec3 axis( 0.0f, 0.0f, 1.0f );

    glm::vec3 pos( 11.0f/6.0f, 0.5f, 0.0f );
    
    glm::vec4 a( 2.0f/3.0f, 1.0f/3.0f, 0.0f, 1.0f );
    glm::vec4 c( 5.0f/3.0f, 5.0f/6.0f, 0.0f, 1.0f );
    
    {
        glm::mat4 s = glm::scale( glm::mat4( 1.0f ), homothetie );
        glm::mat4 r = glm::rotate( glm::mat4( 1.0f ), angle, axis );
        glm::mat4 t = glm::translate( glm::mat4( 1.0f ), pos );
        glm::mat4 mc = t * (r * s);

        glm::vec4 cal_c = mc * a;
        EXPECT_VEC4_NEAR( c, cal_c, precision );
    }

    {
        Orientation s;
        s._homothetie = homothetie;
        Orientation r;
        r._rotation = glm::angleAxis( angle, axis );
        Orientation t;
        t._position = pos;

        Orientation qc = t * (r * s);

        // Near except
        glm::vec3 cal_c = qc * a;
        EXPECT_VEC3_NEAR( c, cal_c, precision );
    }
}

TEST( GLM, quaternion_SRT_non_uniform )
{
    float angle = PI / 2.0f;
    const glm::vec3 axis( 0.0f, 0.0f, 1.0f );

    const glm::vec3 pos( 11.0f/6.0f, 0.5f, 0.0f );

    const glm::vec3 homothetie( 0.5f, 1.0f, 0.5f );

    const glm::vec4 a( 4.0f/6.0f, 2.0f/6.0f, 0.0f, 1.0f );
    const glm::vec4 c( 9.0f/6.0f, 5.0f/6.0f, 0.0f, 1.0f );

    {
        glm::mat4 s = glm::scale( glm::mat4( 1.0f ), homothetie );
        glm::mat4 r = glm::rotate( glm::mat4( 1.0f ), angle, axis );
        glm::mat4 t = glm::translate( glm::mat4( 1.0f ), pos );
        glm::mat4 mc = t * (r * s);

        glm::vec4 cal_c = mc * a;
        EXPECT_VEC4_NEAR( c, cal_c, precision );
    }

    {
        Orientation s;
        s._homothetie = homothetie;
        Orientation r;
        r._rotation = glm::angleAxis( angle, axis );
        Orientation t;
        t._position = pos;

        Orientation qc = t * (r * s);

        // Near except
        glm::vec3 cal_c = qc * a;
        EXPECT_VEC3_NEAR( c, cal_c, precision );
    }
}

TEST( GLM, quaternion_final_composition )
{
    float angleM =  PI / 2.0f;
    float angleR = -PI / 4.0f;
    const glm::vec3 axis( 0.0f, 0.0f, 1.0f );

    const glm::vec4 a( 1.0f, 0.0f, 0.0f, 1.0f );
    const glm::vec4 c( 1.0f, 7.0f/3.0f, 0.0f, 1.0f );


    Orientation qm;
    qm._homothetie = glm::vec3( 0.5f, 1.0f, 1.0f );
    qm._rotation   = glm::angleAxis( angleM, axis );
    qm._position   = glm::vec3(2.0f/3.0f, 1.0f/2.0f, 0.0f);

    Orientation qr;
    qr._homothetie = glm::vec3( sqrt_2 * 2.0f/3.0f, sqrt_2 / 3.0f, 1.0f );
    qr._rotation   = glm::angleAxis( angleR, axis );
    qr._position   = glm::vec3(3.0f, 0.0f, 0.0f);

    EXPECT_VEC3_NEAR( glm::vec3(2.0f/3.0f, 2.0f, 0.0f),
                        qm * qr._position, precision );

    // Composition
    Orientation qc = qm * qr;

    // Near except
    glm::vec3 cal_c = qc * a;
    EXPECT_VEC3_NEAR( c, cal_c, precision );
}

TEST( GLM, quaternion_interpolate )
{
    float angle = PI;
    const glm::vec3 axis( 0.0f, 0.0f, 1.0f );

    const glm::vec4 a( 1.0f, 1.0f, 0.0f, 1.0f );

    Orientation qo;

    Orientation qm;
    qm._homothetie = glm::vec3( 2.0f, 2.0f, 2.0f );
    qm._rotation   = glm::angleAxis( angle, axis );
    qm._position   = glm::vec3(4.0f, 4.0f, 0.0f);

    const int animation = 30;
    for( int i=0; i < animation; ++i)
    {
        float f = float( i ) / float( animation - 1 );
        Orientation qc = Orientation::slerp( qo, qm, f );

        // Near except
        glm::vec3 cal_c = qc * a;

        std::cout << cal_c.x << "," << cal_c.y << std::endl;
    }

    Orientation qc = Orientation::slerp( qo, qm, 0.5f );

    // Near except
    {
        const glm::vec4 c( 3.5f, 0.5f, 0.0f, 1.0f );
        glm::vec3 cal_c = qc * a;
        EXPECT_VEC3_NEAR( c, cal_c, precision );
    }
}