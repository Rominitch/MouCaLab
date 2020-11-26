#pragma once

#include <LibRT/include/RTViewport.h>
#include <LibRT/include/RTWindow.h>

namespace MouCaGraphic
{
    class GraphicConfiguration final
    {
        MOUCA_NOCOPY_NOMOVE(GraphicConfiguration);

        public:
            struct ConfigurationWindow
            {
                RT::ViewportInt32 _screenRect;      ///< Size and position of window into Desktop.
                glm::uvec2        _resolution;      ///< Resolution of RenderBuffer (prefer lower or equal to _screenRect sizes)
                RT::Window::Mode  _mode;
            };
            using ConfigurationWindows = std::vector<ConfigurationWindow>;

                        
            GraphicConfiguration():
            _version(1.0f)
            {}

            ~GraphicConfiguration() = default;

            void clear()
            {
                _windows.clear();
            }

            void addWindow(const ConfigurationWindow& window)
            {
                _windows.emplace_back(window);
            }

        //--------------------------------------------------------------------------
        //						Getter/Setter methods
        //--------------------------------------------------------------------------
            const ConfigurationWindows& getWindowConfiguration() const
            {
                return _windows;
            }

        protected:
            const float			    _version;
            ConfigurationWindows    _windows;   ///< Screen definition by window
            
    };
};