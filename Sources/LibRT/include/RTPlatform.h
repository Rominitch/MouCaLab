/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTVirtualMouse.h>

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Main abstract platform of windows / inputs.
    class Platform
    {
        MOUCA_NOCOPY_NOMOVE(Platform);

        public:
            //------------------------------------------------------------------------
            /// \brief Destructor: need to call release() before to use it.
            /// 
            virtual ~Platform() = default;

            //------------------------------------------------------------------------
            /// \brief  Get readable virtual mouse.
            /// 
            /// \returns Virtual mouse.
            virtual const RT::VirtualMouse& getMouse() const = 0;
            
            //------------------------------------------------------------------------
            /// \brief  Get editable virtual mouse.
            /// 
            /// \returns Editable virtual mouse.
            virtual RT::VirtualMouse& getEditMouse() = 0;

            //------------------------------------------------------------------------
            /// \brief  Check if windows is present into platform.
            /// 
            /// \returns True is windows inside, false otherwise.
            virtual bool isWindowsActive() = 0;

            //------------------------------------------------------------------------
            /// \brief  Run events for this cycle.
            /// 
            virtual void pollEvents() = 0;

        protected:            

            //------------------------------------------------------------------------
            /// \brief Constructor: need to call initialize() after to use it.
            /// 
            Platform() = default;
    };
}