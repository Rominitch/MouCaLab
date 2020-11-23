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
    class Locale;
    using LocaleWPtr = std::weak_ptr<Locale>;
    using LocaleSPtr = std::shared_ptr<Locale>;

    class ErrorLibrary;
    class Exception;

    class ErrorPrinter;
    using ErrorPrinterPtr = std::shared_ptr<ErrorPrinter>;

    //----------------------------------------------------------------------------
    /// \brief Allow to manage message of error based on library/internationalization and error printer.
    ///
    /// \see   
    class ErrorManager final
    {
        public:
            /// Constructor
            ErrorManager();

            /// Destructor
            ~ErrorManager();
            
            //------------------------------------------------------------------------
            /// \brief  Register a printer into manager.
            /// 
            /// \param[in]  printer: link to printer.
            void setPrinter(const ErrorPrinterPtr& printer)
            {
                BT_ASSERT(printer != nullptr);
                _printer = printer;
            }

            void setConfiguration(const LocaleWPtr& locale)
            {
                _linkLocale = locale;
            }

        //--------------------------------------------------------------------------------------
        //								Library Management methods
        //--------------------------------------------------------------------------------------
            void addErrorLibrary(XML::Parser& parser, const String& strLibraryName);

        //--------------------------------------------------------------------------------------
        //								Error Management methods
        //--------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Print exception into registered printer.
            /// 
            /// \param[in] exception: data to print.
            void show(const Exception& exception) const;

            //------------------------------------------------------------------------
            /// \brief  Get final string based on ErrorData information.
            /// 
            /// \param[in] error: current information of error.
            /// \returns Final message in good language.
            /// \note If library is not registered, just print library/error code.
            String getError(const ErrorData& error) const;

        private:
            std::map<String, ErrorLibrary*>	_mapErrorsLibrary;  ///< Collection of libraries' errors.
            LocaleWPtr                      _linkLocale;      ///< Link to internationalization: NEVER delete.
            ErrorPrinterPtr					_printer;           ///< Link to printer.

            //------------------------------------------------------------------------
            /// \brief  Clean manager.
            /// 
            void release();

            MOUCA_NOCOPY_NOMOVE(ErrorManager);
    };
}