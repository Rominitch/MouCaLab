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

            const std::vector<RT::Monitor>& getMonitors() const
            {
                return _monitors;
            }

        private:
            std::vector<RT::Monitor> _monitors; ///< List of monitors
    };
}