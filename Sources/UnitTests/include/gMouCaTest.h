/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    using Path = std::filesystem::path;
}

class MouCaEnvironment : public testing::Environment
{
    private:
        static Core::Path g_inputPath;
        static Core::Path g_outputPath;
        static Core::Path g_workingPath;

        static bool _isDemonstrator;     ///< Enable playable demo.

    public:
        void initialize(int argc, char** argv)
        {
            MOUCA_ASSERT(argc > 0);

            //Read application path
            Core::Path path;
            if(argc > 0)
            {
                path = Core::Path(argv[0]).parent_path();
            }
            MOUCA_ASSERT(std::filesystem::is_directory(path));

            g_inputPath   = Core::Path(path.wstring()) / L".." / L"Inputs";
            g_outputPath  = Core::Path(path.wstring()) / L".." / L"Outputs";
            g_workingPath = Core::Path(path.wstring());

            // Try to create Outputs folder (needed from scratch)
            std::filesystem::create_directories(g_outputPath);

            for (int id = 1; id < argc; ++id)
            {
                if (Core::String(argv[id]).compare("--demonstrator") == 0)
                {
                    _isDemonstrator = true;
                }
            }

            //Post condition all data must be valid !
            MOUCA_ASSERT(std::filesystem::is_directory(getInputPath()));
            MOUCA_ASSERT(std::filesystem::is_directory(getOutputPath()));
        }

        static const Core::Path& getInputPath()
        {
            return g_inputPath;
        }

        static const Core::Path& getOutputPath()
        {
            return g_outputPath;
        }

        static const Core::Path& getWorkingPath()
        {
            return g_workingPath;
        }

        //------------------------------------------------------------------------
        /// \brief  Activate demonstrator mode using "--demonstrator"
        /// 
        /// \returns True if demonstrator is activated
        static bool isDemonstrator()
        {
            return _isDemonstrator;
        }
};

//-------------------------------------------------------------------------------------------------
//                                         GoogleTest Extended
//-------------------------------------------------------------------------------------------------
namespace testing
{
    template <typename T1, typename T2>
    AssertionResult AssertBetweenInclusive( const char*, const char*, const char*, const T1& a, const T1& b, const T2& val )
    {
        if( (static_cast<T2>(a) <= val) && (val <= static_cast<T2>(b)) )
            return AssertionSuccess();
        else
            return AssertionFailure() << val << " is outside the range " << a << " to " << b;
    }
}
#define EXPECT_BETWEEN(minV, maxV, val1)            EXPECT_PRED_FORMAT3(testing::AssertBetweenInclusive, minV, maxV, val1)

//-------------------------------------------------------------------------------------------------
//                                         GoogleTest with RT
//-------------------------------------------------------------------------------------------------
#ifdef RT_AVAILABLE
#   include <LibRT/include/RTViewport.h>

namespace testing
{

    AssertionResult AssertVec2Near( const char* e1, const char* e2, const char* e3,
                                    const RT::Point2& v1, const RT::Point2& v2, const float& v3 );

    AssertionResult AssertVec4Near( const char* e1, const char* e2, const char* e3,
                                    const RT::Point4& v1, const RT::Point4& v2, const float& v3 );

    AssertionResult AssertViewportEq( const char* e1, const char* e2,
                                     const RT::ViewportInt32& v1, const RT::ViewportInt32& v2 );
}

#   define EXPECT_VEC2_NEAR(val1, val2, abs_error)     EXPECT_PRED_FORMAT3(testing::AssertVec2Near, val1, val2, abs_error)
#   define EXPECT_VEC4_NEAR(val1, val2, abs_error)     EXPECT_PRED_FORMAT3(testing::AssertVec4Near, val1, val2, abs_error)
#   define EXPECT_VIEWPORT_EQ(val1, val2)              EXPECT_PRED_FORMAT2(testing::AssertViewportEq, val1, val2)
#endif

//-------------------------------------------------------------------------------------------------
//                                         GoogleTest with Boost
//-------------------------------------------------------------------------------------------------
#ifdef BOOST_AVAILABLE

#define GTEST_TEST_BOOST_NO_THROW_(statement, fail) \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
  if (::testing::internal::AlwaysTrue()) { \
    try { \
      GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement); \
    } \
    catch (const boost::system::system_error & e) { \
      std::stringstream ss; \
      ss << "BOOST Exception: " << e.what(); \
      std::cout << ss.str() << std::endl; \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testnothrow_, __LINE__); \
    } \
    catch (...) { \
      OutputDebugString(L"No BOOST exception"); \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testnothrow_, __LINE__); \
    } \
  } else \
    GTEST_CONCAT_TOKEN_(gtest_label_testnothrow_, __LINE__): \
      fail("Expected: " #statement " doesn't throw an exception.\n" \
           "  Actual: it throws.")

#define EXPECT_BOOST_NO_THROW(statement) \
  GTEST_TEST_BOOST_NO_THROW_(statement, GTEST_NONFATAL_FAILURE_)

#endif