/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    class ErrorLibrary;

    //----------------------------------------------------------------------------
    /// \brief Abstract class to print error message anywhere (windows, console, ...).
    ///
    class ErrorPrinter
    {
        public:
            /// Destructor
            virtual ~ErrorPrinter() = default;

            //------------------------------------------------------------------------
            /// \brief  Print error data into support.
            /// 
            /// \param[in] pError: error data to print
            /// \param[in] pLibrary: library which contains error.
            virtual void print(const Core::ErrorData& pError, const Core::ErrorLibrary* pLibrary) const = 0;

        protected:
            /// Constructor
            ErrorPrinter() = default;

        private:
            MOUCA_NOCOPY_NOMOVE(ErrorPrinter);
    };

    using ErrorPrinterPtr = std::shared_ptr<ErrorPrinter>;
}