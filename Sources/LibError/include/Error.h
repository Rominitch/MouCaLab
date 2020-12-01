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

    class Exception;

    class ErrorPrinter;
    using ErrorPrinterPtr = std::shared_ptr<ErrorPrinter>;

    class IErrorManager
    {
        MOUCA_NOCOPY_NOMOVE(IErrorManager);

        public:
            /// Destructor
            virtual ~IErrorManager() = default;

            //------------------------------------------------------------------------
            /// \brief  Register a printer into manager.
            /// 
            /// \param[in]  printer: link to printer.
            virtual void setPrinter(const ErrorPrinterPtr& printer) = 0;

            virtual void setConfiguration(const LocaleWPtr& locale) = 0;

            //--------------------------------------------------------------------------------------
            //								Library Management methods
            //--------------------------------------------------------------------------------------
            virtual void addErrorLibrary(XML::Parser& parser, const String& strLibraryName) = 0;

            //--------------------------------------------------------------------------------------
            //								Error Management methods
            //--------------------------------------------------------------------------------------
                //------------------------------------------------------------------------
                /// \brief  Print exception into registered printer.
                /// 
                /// \param[in] exception: data to print.
            virtual void show(const Exception& exception) const = 0;

            //------------------------------------------------------------------------
            /// \brief  Get final string based on ErrorData information.
            /// 
            /// \param[in] error: current information of error.
            /// \returns Final message in good language.
            /// \note If library is not registered, just print library/error code.
            virtual String getError(const ErrorData& error) const = 0;

        protected:
            /// Constructor
            IErrorManager() = default;
    };
}