/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace GUI
{
    class Widget;
    using WidgetSPtr = std::shared_ptr<Widget>;

    ///	\brief	Page is a container of widgets (text, button, ...).
    ///         It allow to refresh each element + manage global matrix. 
    class Page
    {
        public:
            Page() = default;
            ~Page() = default;

            void addWidget(WidgetSPtr widget);

            const std::vector<WidgetSPtr>& getWidgets() const { return _widgets; }

        private:
            std::vector<WidgetSPtr> _widgets;
    };
};