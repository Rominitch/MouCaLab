/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTViewport.h>
namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Store monitor info give by hardware reader.
    class Monitor
    {
        public:
            enum ScreenRatio
            {
                Screen4_3,
                Screen16_9,
                Screen16_10,
                ScreenUndefined
            };
            
            /// Constructor
            Monitor():
            _screenRatio(ScreenUndefined), _refreshRate(0), _handle(nullptr)
            {}
        
            /// Destructor
            ~Monitor() = default;

            void initialize(const Core::String name, const RT::ViewportInt32& monitorArea, const RT::Array2& physicSize, const uint32_t refreshRate, void* handle)
            {
                MOUCA_PRE_CONDITION(monitorArea.getHeight() > 0 && monitorArea.getWidth() > 0);

                _name = name;
                _monitorArea = monitorArea;
                _physicSize  = physicSize;

                _refreshRate    = refreshRate;
                _handle         = handle;

                const float radio = static_cast<float>(_monitorArea.getWidth())/static_cast<float>(_monitorArea.getHeight());
                if(fabs(radio-16.0f/9.0f) < 1e-5f)
                {
                    _screenRatio = Screen16_9;
                }
                else if(fabs(radio-16.0f/10.0f) < 1e-5f)
                {
                    _screenRatio = Screen16_10;
                }
                else if(fabs(radio-4.0f/3.0f) < 1e-5f)
                {
                    _screenRatio = Screen4_3;
                }
                else
                {
                    _screenRatio = ScreenUndefined;
                }
            }

        //-----------------------------------------------------------------------------------------
        //                                 Getter/Setter methods
        //-----------------------------------------------------------------------------------------

            const Core::String& getName() const
            {
                return _name;
            }

            const RT::Array2& getPhysicSize() const
            {
                return _physicSize;
            }

            const RT::ViewportInt32& getMonitorArea() const
            {
                return _monitorArea;
            }

            void* getHandle() const
            {
                return _handle;
            }

            const uint32_t getRefreshRate() const
            {
                return _refreshRate;
            }

            ScreenRatio getScreenRatio() const
            {
                return _screenRatio;
            }

        private:
            ScreenRatio	        _screenRatio;
            RT::ViewportInt32	_monitorArea;   ///< Monitor position/size in coordinate space (NOT PIXEL). 
            Core::String          _name;
            RT::Array2          _physicSize;    ///< Physical size in mm

            uint32_t          _refreshRate;
            void*               _handle;
    };
}