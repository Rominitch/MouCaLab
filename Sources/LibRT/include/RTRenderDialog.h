/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTCanvas.h>
#include <LibRT/include/RTWindow.h>

namespace RT
{
    class RenderDialog : public RT::Window, public RT::Canvas
    {
        public:
            RenderDialog() = default;
            ~RenderDialog() override = default;
    };
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;
    using RenderDialogSPtr = std::shared_ptr<RenderDialog>;
}

namespace std
{
    template<> struct less<RT::RenderDialogWPtr>
    {
        bool operator() (const RT::RenderDialogWPtr& lhs, const RT::RenderDialogWPtr& rhs) const
        {
            return lhs.lock() < rhs.lock();
        }
    };
}