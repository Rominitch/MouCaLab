/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreString.h>

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Main class for International Language property.
    ///
    class Locale
    {
        MOUCA_NOCOPY_NOMOVE(Locale);

        public:
            const static String defaultLanguage;    ///< Default language: fr
            const static String defaultCountry;     ///< Default language: FR

            /// Constructor
            Locale();

            /// Destructor
            ~Locale() = default;

            //------------------------------------------------------------------------
            /// \brief  Get current language code.
            /// 
            /// \returns  Language code.
            const String& getLanguage() const   { return _language; }

            //------------------------------------------------------------------------
            /// \brief  Get current country code.
            /// 
            /// \returns  Country code.
            const String& getCountry() const { return _country; }

            //------------------------------------------------------------------------
            /// \brief  Set language code.
            /// 
            /// \param[in] language: new language code.
            void setLanguage(const String& language)
            {
                _language = language;
            }

            //------------------------------------------------------------------------
            /// \brief  Set country code.
            /// 
            /// \param[in] language: new country code.
            void setCountry(const String& country)
            {
                _country = country;
            }

        protected:
            String _language;   ///< Language code.
            String _country;    ///< Country  code.
    };
}