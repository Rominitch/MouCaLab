/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTEnum.h>
#include <LibRT/include/RTVirtualMouse.h>

namespace RT
{
    class Canvas;
    class Viewer;

    class EventManager
    {
        MOUCA_NOCOPY_NOMOVE(EventManager);

        public:
            virtual ~EventManager() = default;

            virtual void onClose(Canvas* pCanvas) = 0;

            virtual void onSize(Canvas* pCanvas, const RT::Array2ui& size) = 0;

            virtual void onMouseMove(Canvas* pCanvas, const RT::VirtualMouse& mouse) = 0;

            virtual void onMousePress(Canvas* pCanvas, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons eButton) = 0;
            virtual void onMouseRelease(Canvas* pCanvas, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons eButton) = 0;

            virtual void onMouseWheel(Canvas* pCanvas, const RT::VirtualMouse& mouse, const int positionRelease) = 0;

            //------------------------------------------------------------------------
            /// \brief  All keyboard action.
            /// 
            /// \param[in] action: see state GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
            virtual void onKeyPress( Canvas* pCanvas, int key, int scancode, int action, int mods ) = 0;

        protected:
            EventManager() = default;

            SMouseState	_mouseState;
    };

    using EventManagerSPtr = std::shared_ptr<EventManager>;
    using EventManagerWPtr = std::weak_ptr<EventManager>;
}