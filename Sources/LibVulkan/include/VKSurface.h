/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKSurfaceFormat.h>

namespace RT
{
    class Window;
}

namespace Vulkan
{
    class Device;
    class Environment;

    class Surface final
    {
        MOUCA_NOCOPY(Surface);

        public:
            Surface();
            ~Surface();

            void initialize(const Environment& environment, const RT::Window& window);

            void release(const Environment& environment);

            bool isNull() const
            {
                return _surface == VK_NULL_HANDLE;
            }

            void computeSurfaceFormat(const Device& device, const SurfaceFormat::Configuration& userPreferences, SurfaceFormat& format);

            const VkSurfaceKHR& getInstance() const
            {
                return _surface;
            }

        private:
            VkSurfaceKHR		_surface;		///< Surface ID.
    };
}