/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreDefine.h>
#include <LibCore/include/CoreString.h>

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Main structure of data for manage kind of error program can launched.
    ///
    /// \see   Exception, ErrorLibrary, ErrorManager
    class ErrorData final
    {
        public:
            enum class Code : uint8_t
            {
                LibraryCode,
                ErrorCode,
                NbCode
            };

            /// Default constructor: build an unknown error.
            ErrorData(const std::source_location& source = std::source_location::current()):
            _sourceLocation(source)
            {
                _errorDefinition[static_cast<size_t>(Code::LibraryCode)] = "BasicError";
                _errorDefinition[static_cast<size_t>(Code::ErrorCode)]   = "UnknownError";
            }

            //------------------------------------------------------------------------
            /// \brief  Constructor of error.
            /// 
            /// \param[in] strLib: library name.
            /// \param[in] strError: error name.
            /// \param[in] strInfo: parameter inside message to substitute.
            /// \param[in] strFile: file where error has been met.
            /// \param[in] iLine: line in source where error has been met.
            ErrorData(const StringView& strLib, const StringView& strError, const std::source_location& source = std::source_location::current()):
            _sourceLocation(source)
            {
                _errorDefinition[static_cast<size_t>(Code::LibraryCode)] = strLib;
                _errorDefinition[static_cast<size_t>(Code::ErrorCode)]   = strError;
            }

            //------------------------------------------------------------------------
            /// \brief  Copy constructor.
            /// 
            /// \param[in] copy: object to copy.
            ErrorData(const ErrorData& copy):
            _sourceLocation(copy._sourceLocation),
            _errorDefinition(copy._errorDefinition),
            _parameters(copy._parameters)
            {}

            /// Destructor
            ~ErrorData(void) = default;

            //------------------------------------------------------------------------
            /// \brief  Get library name.
            /// 
            /// \returns Library name.
            const StringView& getLibraryLabel() const
            {
                return _errorDefinition[0];
            }

            //------------------------------------------------------------------------
            /// \brief  Get error name.
            /// 
            /// \returns Error name.
            const StringView& getErrorLabel() const
            {
                return _errorDefinition[1];
            }

            //------------------------------------------------------------------------
            /// \brief  Search parameters into message and replace by corresponding info.
            /// 
            /// \param[in] strMessage: error message with %P0 to replace.
            /// \returns New message without %P0, %P1, ...
            String convertMessage(const String& strMessage) const;

            //------------------------------------------------------------------------
            /// \brief  Get where exception is launched.
            /// 
            /// \returns File name.
            const std::source_location& getSourceLocation() const
            {
                return _sourceLocation;
            }

            //------------------------------------------------------------------------
            /// \brief  Get number of parameters which has been register.
            /// 
            /// \returns Number of parameters.
            size_t getNbParameters() const
            {
                return _parameters.size();
            }

            //------------------------------------------------------------------------
            /// \brief  Get specific parameters.
            /// 
            /// \param[in] szID: parameter position into array.
            /// \note Crash if overflow
            /// \returns Parameter at specific position.
            String getParameters(const size_t szID) const
            {
                return _parameters[szID];
            }

            const std::vector<String>& getParameters() const { return _parameters; }

            ErrorData& operator<<(const Core::String& data)
            {
                _parameters.push_back(data);
                return *this;
            }

        private:
            std::array<StringView, static_cast<size_t>(Code::NbCode)> _errorDefinition;    ///< Error definition: Library + ErrorCode
            std::vector<String>      _parameters;             ///< Parameters list for substitute in message.

            std::source_location     _sourceLocation;
    };
}