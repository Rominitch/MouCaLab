/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace XML
{
    class Parser;
}

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Data which describe error for user in this language.
    ///
    class ErrorDescription final
    {
        public:
            /// Constructor
            //ErrorDescription() = default;

            /// Destructor
            ~ErrorDescription() = default;

            //------------------------------------------------------------------------
            /// \brief Main constructor which produce valid error.
            /// 
            /// \param[in] strMessage: .
            /// \param[in] strSolution: .
            ErrorDescription(const StringView& strMessage, const StringView& strSolution):
            _message(strMessage), _solution(strSolution)
            {}

            //------------------------------------------------------------------------
            /// \brief  Get error message in good language but with undefined parameters.
            /// 
            /// \returns Error message.
            const String& getMessage() const
            {
                return _message;
            }

            //------------------------------------------------------------------------
            /// \brief  Get solution message in good language but with undefined parameters.
            /// 
            /// \returns Solution message.
            const String& getSolution() const
            {
                return _solution;
            }

        private:
            String	_message;   ///< Contains error message for user in good language.
            String	_solution;  ///< Contains solution for user in good language.

            MOUCA_NOCOPY_NOMOVE(ErrorDescription);
    };

    //----------------------------------------------------------------------------
    /// \brief Define a collection of ErrorDescription.
    ///
    /// \see   
    class ErrorLibrary final
    {
        public:
            ErrorLibrary(void) = default;
            ~ErrorLibrary(void);

            void release();

            void initialize(XML::Parser& parser, const StringView& strLibraryLabel, const String& strCountry);

            ErrorDescription const * const getDescription(const String& strLabel) const;

        private:
            std::map<String, ErrorDescription const *> _mapErrors;	///< Link between label error and description

            MOUCA_NOCOPY_NOMOVE(ErrorLibrary);
    };
}