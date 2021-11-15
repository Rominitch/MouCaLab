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
            ErrorData():
            _filePath(), _line(0)
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
            ErrorData(const String& strLib, const String& strError, const Path& filePath, const int iLine):
            _filePath(filePath), _line(iLine)
            {
                _errorDefinition[static_cast<size_t>(Code::LibraryCode)] = strLib;
                _errorDefinition[static_cast<size_t>(Code::ErrorCode)]   = strError;
            }

            //------------------------------------------------------------------------
            /// \brief  Copy constructor.
            /// 
            /// \param[in] copy: object to copy.
            ErrorData(const ErrorData& copy):
            _filePath(copy._filePath), _line(copy._line),
            _errorDefinition(copy._errorDefinition),
            _parameters(copy._parameters)
            {}

            /// Destructor
            ~ErrorData(void) = default;

            //------------------------------------------------------------------------
            /// \brief  Get library name.
            /// 
            /// \returns Library name.
            const String& getLibraryLabel() const
            {
                return _errorDefinition[0];
            }

            //------------------------------------------------------------------------
            /// \brief  Get error name.
            /// 
            /// \returns Error name.
            const String& getErrorLabel() const
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
            /// \brief  Get file name.
            /// 
            /// \returns File name.
            const Path& getFile() const
            {
                return _filePath;
            }

            //------------------------------------------------------------------------
            /// \brief  Get line.
            /// 
            /// \returns Line.
            int getLine() const
            {
                return _line;
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
            std::array<String, static_cast<size_t>(Code::NbCode)> _errorDefinition;    ///< Error definition: Library + ErrorCode

            Path                     _filePath;               ///< Source code where error is launched.
            int                      _line;                   ///< Line position in source code where error is launched.
            std::vector<String>      _parameters;             ///< Parameters list for substitute in message.
    };
}