/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

/// Assertion 
#ifndef NDEBUG
#   define BT_ASSERT_HEADER(condition, header)                                                                                  \
    if(!static_cast<bool>(condition))                                                                                           \
    {                                                                                                                           \
        Core::String message;                                                                                                   \
        message += Core::String(__FILE__) + Core::String(u8"(") + std::to_string(__LINE__)                                      \
                + Core::String(u8"): [ERROR] ")+ Core::String(#header) + Core::String(u8" - ")                                  \
                + Core::String(#condition) + Core::String(u8"\n");                                                              \
        std::cerr << message;                                                                                                   \
        OutputDebugString(Core::convertToOS(message).c_str());                                                                  \
        assert(condition);                                                                                                      \
    }

#   define BT_PRE_CONDITION(condition)  BT_ASSERT_HEADER(condition, u8"Pre-condition assert")
#   define BT_POST_CONDITION(condition) BT_ASSERT_HEADER(condition, u8"Post-condition assert")

#   define BT_ASSERT(condition) BT_ASSERT_HEADER(condition, u8"Application assert")

#   define BT_LAST_REFERENCED(iterable)                                                                                         \
    for(const auto& iter : iterable)                                                                                            \
    {                                                                                                                           \
        BT_ASSERT(iter.use_count() == 1);                                                                                       \
    }

#   define BT_NOT_LAST_REFERENCED(iterable)                                                                                     \
    for(const auto& iter : iterable)                                                                                            \
    {                                                                                                                           \
        BT_ASSERT(iter.use_count() != 1);                                                                                       \
    }

#   define BT_ASSERT_EQ(ref, cmp)                                                                                               \
    if(ref != cmp)                                                                                                              \
    {                                                                                                                           \
        Core::String message;                                                                                                   \
        message += Core::String(__FILE__) + Core::String(u8"(") + std::to_string(__LINE__)                                      \
                + Core::String(u8"): [ERROR] Application assert - Compare:\n")                                                  \
                + Core::String(u8"[ERROR] ") + Core::String(#ref) + Core::String(u8" = ") + std::to_string(ref) + Core::String(u8"\n")  \
                + Core::String(u8"[ERROR] ") + Core::String(#cmp) + Core::String(u8" = ") + std::to_string(cmp) + Core::String(u8"\n"); \
        std::cerr << message;                                                                                                   \
        OutputDebugString(Core::convertToOS(message).c_str());                                                                  \
        assert(ref == cmp);                                                                                                     \
    }

/*
template<typename DataType>
void assertBetween(const DataType ref, const DataType lowEd, const DataType HighTh,
                   const Core::String& refName, const Core::String& lowName, const Core::String& highName,
                   const Core::String& file, const int32_t line)
{
    if (lowEd > ref || ref >= HighTh)                                                                             
    {                                                                                                             
        Core::String message;                                                                                       
        message += file + Core::String(u8"(") + std::to_string(line)
        + Core::String(u8"): [ERROR] Application assert - Between:\n")
        + Core::String(u8"[ERROR] ") + lowName + Core::String(u8" <= ") + refName
        + Core::String(u8" < ") + highName + Core::String(u8"\n")
        + Core::String(u8"[ERROR] ") + std::to_string(lowEd) + Core::String(u8" <= ") + std::to_string(ref)
        + Core::String(u8" < ") + std::to_string(HighTh) + Core::String(u8"\n");
        std::cerr << message;                                                                                                  
        OutputDebugString(Core::convertToOS(message).c_str());                                                                   
        assert(lowEd <= ref && ref < HighTh);                                                                                  
    }
}

template<>
void assertBetween(const uint8_t ref, const uint8_t lowEd, const uint8_t HighTh,
    const Core::String& refName, const Core::String& lowName, const Core::String& highName,
    const Core::String& file, const int32_t line)
{
    if (lowEd > ref || ref >= HighTh)                                                                                          
    {                                                                                                                          
        Core::String message;                                                                                                    
        message += file + Core::String(u8"(") + std::to_string(line)
        + Core::String(u8"): [ERROR] Application assert - Between:\n")
        + Core::String(u8"[ERROR] ") + lowName + Core::String(u8" <= ") + refName
        + Core::String(u8" < ") + highName + Core::String(u8"\n")
        + Core::String(u8"[ERROR] ") + std::to_string(static_cast<int>(lowEd)) + Core::String(u8" <= ") + std::to_string(static_cast<int>(ref))
        + Core::String(u8" < ") + std::to_string(static_cast<int>(HighTh)) + Core::String(u8"\n");                                          
    std::cerr << message;
    OutputDebugString(Core::convertToOS(message).c_str());
    assert(lowEd <= ref && ref < HighTh);
    }
}
 #   define BT_ASSERT_BETWEEN(ref, lowEd, HighTh)                                                                                \
        assertBetween(ref, lowEd, HighTh, #ref, #lowEd, #HighTh, __FILE__, __LINE__);
*/
#   define BT_ASSERT_BETWEEN(ref, lowEd, HighTh)                                                                                \
    if(lowEd > ref || ref >= HighTh)                                                                                            \
    {                                                                                                                           \
        Core::String message;                                                                                                   \
        message += Core::String(__FILE__) + Core::String(u8"(") + std::to_string(__LINE__)                                      \
                + Core::String(u8"): [ERROR] Application assert - Between:\n")                                                  \
                + Core::String(u8"[ERROR] ") + Core::String(#lowEd) + Core::String(u8" <= ") + Core::String(#ref)               \
                + Core::String(u8" < ") + Core::String(#HighTh) + Core::String(u8"\n")                                          \
                + Core::String(u8"[ERROR] ") + std::to_string(static_cast<int>(lowEd)) + Core::String(u8" <= ") + std::to_string(static_cast<int>(ref)) \
                + Core::String(u8" < ") + std::to_string(static_cast<int>(HighTh)) + Core::String(u8"\n");                      \
        std::cerr << message;                                                                                                   \
        OutputDebugString(Core::convertToOS(message).c_str());                                                                  \
        assert(lowEd <= ref && ref < HighTh);                                                                                   \
    }

#   define BT_ASSERT_BETWEEN_EQ(ref, lowEq, HighEq)                                                                             \
    if(lowEq > ref || ref > HighEq)                                                                                             \
    {                                                                                                                           \
        Core::String message;                                                                                                   \
        message += Core::String(__FILE__) + Core::String(u8"(") + std::to_string(__LINE__)                                      \
                + Core::String(u8"): [ERROR] Application assert - Between:\n")                                                  \
                + Core::String(u8"[ERROR] ") + Core::String(#lowEq) + Core::String(u8" <= ") + Core::String(#ref)               \
                + Core::String(u8" <= ") + Core::String(#HighEq) + Core::String(u8"\n")                                         \
                + Core::String(u8"[ERROR] ") + std::to_string(lowEq) + Core::String(u8" <= ") + std::to_string(ref)             \
                + Core::String(u8" <= ") + std::to_string(HighEq) + Core::String(u8"\n");                                       \
        std::cerr << message;                                                                                                   \
        OutputDebugString(Core::convertToOS(message).c_str());                                                                  \
        assert(lowEq <= ref && ref <= HighEq);                                                                                  \
    }

#   define BT_PRINT_MESSAGE(message)                                                                                            \
    {                                                                                                                           \
        std::cerr << message;                                                                                                   \
        OutputDebugString(Core::convertToOS(message).c_str());                                                                  \
    }

#   define BT__STR(x)   #x
#   define BT_STR(x)    BT__STR(x)
    /// Allow to define all TODO into code and retrieve quickly
#   define BT_TODO(msg)                                                  \
    {                                                                    \
        __pragma(message(__FILE__ "(" BT_STR(__LINE__) "): TODO: " msg)) \
    }

#   define BT_DEBUG(msg) std::cout << msg << std::endl

#else
#   define BT_PRE_CONDITION(condition)      void(0)
#   define BT_POST_CONDITION(condition)     void(0)
#   define BT_ASSERT(condition)             void(0)
#   define BT_ASSERT_HEADER(condition, h)   void(0)

#   define BT_ASSERT_EQ(ref, cmp)            void(0)
#   define BT_ASSERT_BETWEEN(ref, le, ht)    void(0)
#   define BT_ASSERT_BETWEEN_EQ(ref, le, he) void(0)
#   define BT_LAST_REFERENCED(iterable)      void(0)
#   define BT_NOT_LAST_REFERENCED(iterable)  void(0)

#   define BT_TODO(msg)                      void(0)    /// Allow to define all TODO into code and retrieve quickly
#   define BT_DEBUG(msg)                     void(0) 
#endif

#define BT_UNUSED(variable) variable;

namespace Core
{
    //using uint8  = uint8_t;     ///< Integer of 8 bits
    //using int8   = int8_t;      ///< Signed Integer of 8 bits
    //using uint16 = uint16_t;    ///< Integer of 16 bits
    //using int16  = int16_t;     ///< Signed Integer of 16 bits
    //using uint32 = uint32_t;    ///< Integer of 32 bits
    //using int32  = int32_t;     ///< Signed  Integer of 32 bits
    //using uint64 = uint64_t;    ///< Integer of 64 bits
    //using int64  = int64_t;     ///< Signed Integer of 64 bits
    //
    //using CPUMemory = size_t;       ///< CPU Memory size

    struct SPictureUC24
    {
        std::array<uint8_t, 3> m_color;
    };

    struct SPictureUC32
    {
        std::array<uint8_t, 4> m_color;
    };

    union ColorUC32
    {
        std::array<uint8_t, 4> m_color = {0,0,0,0};
        struct RGBA
        {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;
            uint8_t a = 0;
        };
    };

    using TimeClock = std::chrono::high_resolution_clock;

    //----------------------------------------------------------------------------
    /// \brief Create a comparator Weak element for set.
    /// \code{.cpp}
    ///     std::set<std::weak_ptr<int>, Core::WeakCompare<int>> mySet;
    /// \endcode
    template<typename DataType>
    struct WeakCompare
    {
        bool operator() (const std::weak_ptr<DataType> &lhs, const std::weak_ptr<DataType> &rhs) const
        {
            BT_PRE_CONDITION(!lhs.expired() && !rhs.expired()); // DEV Issue: Supposed impossible !
            return lhs.lock() < rhs.lock();
        }
    };
}

//Define operator for short
inline int16_t operator "" _i16 (unsigned long long int number)
{
    return static_cast<int16_t>(number);
}

#define BT_NOMOVE(ClassName)                                \
           ClassName(ClassName&&) = delete;                 \
           ClassName& operator=(ClassName&&) = delete

#define BT_DEFAULTMOVE(ClassName)                           \
           ClassName(ClassName&&) = default;                \
           ClassName& operator=(ClassName&&) = default

#define MOUCA_NOCOPY(ClassName)                                \
           ClassName(const ClassName&) = delete;            \
           ClassName& operator=(const ClassName&) = delete

#define MOUCA_NOCOPY_NOMOVE(ClassName)                      \
           MOUCA_NOCOPY(ClassName);                            \
           BT_NOMOVE(ClassName)

#define MOUCA_NOCOPY_DEFAULTMOVE(ClassName)                    \
           MOUCA_NOCOPY(ClassName);                            \
           BT_DEFAULTMOVE(ClassName)