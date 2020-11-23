#include "Dependancies.h"

#include "LibGUI/include/GUIWidget.h"
#include "LibGUI/include/GUIPage.h"

namespace GUI
{

void Page::addWidget(WidgetSPtr widget)
{
    BT_PRE_CONDITION(widget != nullptr);
    BT_PRE_CONDITION(std::find(_widgets.cbegin(), _widgets.cend(), widget) == _widgets.cend());

    _widgets.emplace_back(widget);
}

}