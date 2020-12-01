/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreDefine.h>
#include <LibCore/include/CoreError.h>

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Allow to make our standard exception.
    ///
    /// \see   
    class Exception final
    {
        public:
            //------------------------------------------------------------------------
            /// \brief Constructor
            /// 
            /// \param[in] pFirstError: first data associated.  
            explicit Exception(const ErrorData& pFirstError) noexcept;

            /// Destructor
            ~Exception() noexcept;

            //------------------------------------------------------------------------
            /// \brief  Add another caught place to current exception.
            /// 
            /// \param[in] pLastError: current data
            void addCatchedError(const ErrorData& pLastError) noexcept;

            //------------------------------------------------------------------------
            /// \brief  Read current stack error.
            /// 
            /// \param[in] szId: where read.
            /// \returns Specific data error.
            /// \note Overflow will crash application.
            const ErrorData& read(const size_t szId) const noexcept;

            //------------------------------------------------------------------------
            /// \brief  Get number of error stacked inside.
            /// 
            /// \returns Number of error.
            size_t getNbErrors() const noexcept;

        private:
            std::deque<ErrorData> _errorsHierarchy;      ///< List of met exceptions (first is most recent)
    };
}

#define MOUCA_THROW_ERROR(strLib, strError)				        throw Core::Exception(Core::ErrorData(strLib, strError, __FILE__, __LINE__));
#define MOUCA_THROW_ERROR_1(strLib, strError, p0)		        throw Core::Exception(Core::ErrorData(strLib, strError, __FILE__, __LINE__) << p0);
#define MOUCA_THROW_ERROR_2(strLib, strError, p0, p1)		    throw Core::Exception(Core::ErrorData(strLib, strError, __FILE__, __LINE__) << p0 << p1);
#define MOUCA_THROW_ERROR_3(strLib, strError, p0, p1, p2)	    throw Core::Exception(Core::ErrorData(strLib, strError, __FILE__, __LINE__) << p0 << p1 << p2);
#define MOUCA_THROW_ERROR_4(strLib, strError, p0, p1, p2, p3)	throw Core::Exception(Core::ErrorData(strLib, strError, __FILE__, __LINE__) << p0 << p1 << p2 << p3);