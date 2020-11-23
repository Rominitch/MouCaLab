/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class EventManager;
    using EventManagerWPtr = std::weak_ptr<EventManager>;

    //----------------------------------------------------------------------------
    /// \brief Abstract notion of support of rendering and interaction.
    /// We split two notions in LibRT: Windows / Canvas.
    /// - Canvas is descriptor of interaction and paint part (layer).
    /// - Windows is more software part.
    /// Typically, final class must implement both in same time: GLFWWindow is a RT:Canvas + RT::Window.
    class Canvas
    {
        protected:
            Array2ui    	    _resolutionPixel;
            EventManagerWPtr    _eventManager;      ///< [LINK] Link to event manager.

        public:
            Canvas();
            virtual ~Canvas();

            bool isNull() const
            {
                return _resolutionPixel == Array2ui(0, 0);
            }

            void initialize(const EventManagerWPtr& pEventManager, const Array2ui& resolution);

            void setEventManager(EventManagerWPtr eventManager)
            {
                _eventManager = eventManager;
            }

            void release();

        //--------------------------------------------------------------------------
        //							Getter/Setter methods					
        //--------------------------------------------------------------------------
            void setResolution(const Array2ui& resolution)
            {
                _resolutionPixel = resolution;
            }

            const Array2ui& getResolution() const
            {
                return _resolutionPixel;
            }
    };

}