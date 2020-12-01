#include "Dependencies.h"

#include <LibGUI/include/GUIWidget.h>

namespace GUI
{

TEST(GUIWidget, API)
{
    Widget widget;
    widget.setName(Core::String(u8"MyWidget"));

    // Default state after build
    EXPECT_TRUE(widget.isState(Widget::State::Visible));
    EXPECT_TRUE(widget.isState(Widget::State::Enabled));
    EXPECT_FALSE(widget.isState(Widget::State::Hover));
    EXPECT_FALSE(widget.isState(Widget::State::Focus));

    // Setter
    widget.setVisible(false);
    widget.setEnabled(false);
    EXPECT_FALSE(widget.isState(Widget::State::Visible));
    EXPECT_FALSE(widget.isState(Widget::State::Enabled));

    widget.setVisible(true);
    widget.setEnabled(true);
    EXPECT_TRUE(widget.isState(Widget::State::Visible));
    EXPECT_TRUE(widget.isState(Widget::State::Enabled));
}

}