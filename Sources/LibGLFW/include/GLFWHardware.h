/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTMonitor.h>

namespace GLFW
{
    /// Read all platforms' hardware information:
    /// - Monitors
    class Hardware
    {
        public:
            Hardware() = default;
            ~Hardware() = default;

            // Create information from scratch.
            void readHardware();

            const RT::Monitors& getMonitors() const
            {
                return _monitors;
            }

        private:
            RT::Monitors _monitors; ///< List of monitors
    };
}