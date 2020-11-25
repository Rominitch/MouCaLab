#include "Dependancies.h"

Core::Path MouCaEnvironment::g_inputPath;
Core::Path MouCaEnvironment::g_outputPath;
Core::Path MouCaEnvironment::g_workingPath;
bool       MouCaEnvironment::_isDemonstrator = false;

#ifdef RT_AVAILABLE

#include <LibRT/include/RTEnum.h>

namespace testing
{

 AssertionResult AssertVec2Near(const char* e1,
                                const char* e2,
                                const char*,
                                const RT::Point2& v1,
                                const RT::Point2& v2,
                                const float& v3)
{
    if(fabs(v1.x - v2.x) < v3 && fabs(v1.y - v2.y) < v3)
    {
        return AssertionSuccess();
    }

    return AssertionFailure() << " comparison vector ("
        << e1 << ", "
        << e2 << ") evaluates to false, where"
        << "\n" << e1 << " evaluates to Vec4(" << v1.x << ", " << v1.y << ")"
        << "\n" << e2 << " evaluates to Vec4(" << v2.x << ", " << v2.y << ")"
        << "\n" " must be near than " << v3;
}

AssertionResult AssertVec4Near(const char* e1,
                                const char* e2,
                                const char*,
                                const RT::Point4& v1,
                                const RT::Point4& v2,
                                const float& v3)
{
    if(fabs(v1.x - v2.x) < v3 && fabs(v1.y - v2.y) < v3 && fabs(v1.z - v2.z) < v3 && fabs(v1.w - v2.w) < v3)
    {
        return AssertionSuccess();
    }

    return AssertionFailure() << " comparison vector ("
        << e1 << ", "
        << e2 << ") evaluates to false, where"
        << "\n" << e1 << " evaluates to Vec4(" << v1.x << ", " << v1.y << ", " << v1.z << ", " << v1.w << ")"
        << "\n" << e2 << " evaluates to Vec4(" << v2.x << ", " << v2.y << ", " << v2.z << ", " << v2.w << ")"
        << "\n" " must be near than " << v3;
}

AssertionResult AssertViewportEq(const char* e1,
                                 const char* e2,
                                 const RT::ViewportInt32& v1,
                                 const RT::ViewportInt32& v2)
{
    if(v1.getX() == v2.getX() && v1.getY() == v2.getY()
    && v1.getWidth() == v2.getWidth() && v1.getHeight() == v2.getHeight())
    {
        return AssertionSuccess();
    }

    return AssertionFailure() << " comparison Viewport ("
        << e1 << ", "
        << e2 << ") evaluates to false, where"
        << "\n" << e1 << " evaluates to Viewport(Pos: " << v1.getX() << ", " << v1.getY() << ", Size: " << v1.getWidth() << "x" << v1.getHeight() << ")"
        << "\n" << e2 << " evaluates to Viewport(Pos: " << v2.getX() << ", " << v2.getY() << ", Size: " << v2.getWidth() << "x" << v2.getHeight() << ")";
}

}

#endif