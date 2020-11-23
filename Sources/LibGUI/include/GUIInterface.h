#pragma once

namespace RT
{
    class ContextExecution;
    class DataEnvironment;
    class Renderer;
}

class Widget;

class GUIInterface
{
    using GUIWidgetVector = std::vector<Widget*>;
    
    protected:
        GUIWidgetVector m_vInterfaceWidgets;

    public:
        GUIInterface(void) = default;
        virtual ~GUIInterface(void) = default;

    //---------------------------------------------------------------
    //					Widget management methods
    //---------------------------------------------------------------
        void AddWidget(Widget* pWidget)
        {
            m_vInterfaceWidgets.push_back(pWidget);
        }

    //---------------------------------------------------------------
    //						Read/Save methods
    //---------------------------------------------------------------
        void ReadInterface();
        void SaveInterface();

    //---------------------------------------------------------------
    //						Rendering methods
    //---------------------------------------------------------------
        virtual void Update(RT::ContextExecution* pContext, RT::Renderer* pRenderer);
        virtual void Draw(RT::DataEnvironment* pEnvironment, RT::Renderer* pRenderer) const;
};
