#include "Dependencies.h"

#include "LibGUI/include/GUIWidget.h"
#include "LibGUI/include/GUIPage.h"

namespace GUI
{

void Page::addWidget(WidgetSPtr widget)
{
    MOUCA_PRE_CONDITION(widget != nullptr);
    MOUCA_PRE_CONDITION(std::find(_widgets.cbegin(), _widgets.cend(), widget) == _widgets.cend());

    _widgets.emplace_back(widget);
}

}