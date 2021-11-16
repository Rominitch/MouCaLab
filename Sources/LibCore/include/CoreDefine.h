/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreString.h>

#ifndef NDEBUG
#   define MOUCA_ACTIVE_ASSERT
#endif

namespace MouCa
{
    void assertHeader(const bool condition, const Core::StringView& header, const std::source_location& location = std::source_location::current());

    void preCondition(const bool condition, const std::source_location& location = std::source_location::current());

    void postCondition(const bool condition, const std::source_location& location = std::source_location::current());

    void assertion(const bool condition, const std::source_location& location = std::source_location::current());

    template<typename DataType>
    void assertCompare(const DataType& reference, const DataType& comparison, const std::source_location& location = std::source_location::current())
    {
#ifdef MOUCA_ACTIVE_ASSERT
        if (reference != comparison)
        {
            auto message = std::format("{} ({}): [ERROR] Application assert into {} - Compare: \n {} != {} \n", location.file_name(), location.line(), location.function_name(), std::to_string(reference), std::to_string(comparison));
            std::cerr << message;

            OutputDebugString(Core::convertToOS(message).c_str());
            assert(reference == comparison);
        }
#endif
    }

    template<typename DataType>
    void logConsole(const DataType& message, const std::source_location& location = std::source_location::current())
    {
#ifdef MOUCA_ACTIVE_ASSERT
        std::cout << message;
        OutputDebugString(Core::convertToOS(message).c_str());
#endif
    }
}

/// Assertion 
#ifndef NDEBUG
#   define MOUCA__STR(x)   #x
#   define MOUCA_STR(x)    MOUCA__STR(x)
#endif

#define MOUCA_UNUSED(variable) variable;
/// Allow to define all TODO into code and retrieve quickly
#   define MOUCA_TODO(msg)                                                  \
    {                                                                       \
        __pragma(message(__FILE__ "(" MOUCA_STR(__LINE__) "): TODO: " msg)) \
    }

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
            MouCa::preCondition(!lhs.expired() && !rhs.expired()); // DEV Issue: Supposed impossible !
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

#define MOUCA_NOCOPY(ClassName)                             \
           ClassName(const ClassName&) = delete;            \
           ClassName& operator=(const ClassName&) = delete

#define MOUCA_NOCOPY_NOMOVE(ClassName)                      \
           MOUCA_NOCOPY(ClassName);                         \
           BT_NOMOVE(ClassName)

#define MOUCA_NOCOPY_DEFAULTMOVE(ClassName)                 \
           MOUCA_NOCOPY(ClassName);                         \
           BT_DEFAULTMOVE(ClassName)