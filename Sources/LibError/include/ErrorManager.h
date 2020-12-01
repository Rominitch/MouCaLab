/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibError/include/Error.h>

namespace Core
{
    class ErrorLibrary;

    //----------------------------------------------------------------------------
    /// \brief Allow to manage message of error based on library/internationalization and error printer.
    ///
    /// \see   
    class ErrorManager final : public IErrorManager
    {
        MOUCA_NOCOPY_NOMOVE(ErrorManager);

        public:
            /// Constructor
            ErrorManager();

            /// Destructor
            ~ErrorManager() override;
            
            //------------------------------------------------------------------------
            /// \brief  Register a printer into manager.
            /// 
            /// \param[in]  printer: link to printer.
            void setPrinter(const ErrorPrinterPtr& printer) override
            {
                MOUCA_ASSERT(printer != nullptr);
                _printer = printer;
            }

            void setConfiguration(const LocaleWPtr& locale) override
            {
                _linkLocale = locale;
            }

        //--------------------------------------------------------------------------------------
        //								Library Management methods
        //--------------------------------------------------------------------------------------
            void addErrorLibrary(XML::Parser& parser, const String& strLibraryName) override;

        //--------------------------------------------------------------------------------------
        //								Error Management methods
        //--------------------------------------------------------------------------------------
            //------------------------------------------------------------------------
            /// \brief  Print exception into registered printer.
            /// 
            /// \param[in] exception: data to print.
            void show(const Exception& exception) const override;

            //------------------------------------------------------------------------
            /// \brief  Get final string based on ErrorData information.
            /// 
            /// \param[in] error: current information of error.
            /// \returns Final message in good language.
            /// \note If library is not registered, just print library/error code.
            String getError(const ErrorData& error) const override;

        private:
            std::map<String, ErrorLibrary*>	_mapErrorsLibrary;  ///< Collection of libraries' errors.
            LocaleWPtr                      _linkLocale;        ///< Link to internationalization: NEVER delete.
            ErrorPrinterPtr					_printer;           ///< Link to printer.

            //------------------------------------------------------------------------
            /// \brief  Clean manager.
            /// 
            void release();
    };
}