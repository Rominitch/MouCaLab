/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{

    class SurfaceFormat final
    {
        MOUCA_NOCOPY(SurfaceFormat);

        public:
            struct Configuration
            {
                uint32_t                      _nbImages;
                VkSurfaceFormatKHR            _format;
                VkExtent2D                    _extent;
                VkImageUsageFlags             _usage;
                VkSurfaceTransformFlagBitsKHR _transform;
                VkPresentModeKHR              _presentMode;

                Configuration() :
                    _nbImages(0),
                    _format({VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR}),
                    _extent({0, 0}),
                    _usage(0),
                    _transform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR),
                    _presentMode(VK_PRESENT_MODE_MAILBOX_KHR)
                {}
            };

            /// Constructor: not initialized
            SurfaceFormat() = default;
            /// Destructor
            ~SurfaceFormat() = default;

            /// \brief Reset and configure surface format based on user then surface preferences.
            /// \param[in] preferred: User preferences (can be invalid).
            /// \param[in] surfaceCapabilities: Surface capabilities.
            /// \param[in] surfaceFormats: Surface supported format.
            /// \param[in] surfacePresentModes: Surface supported present mode.
            void initialize(const Configuration& preferred,
                            const VkSurfaceCapabilitiesKHR&         surfaceCapabilities, 
                            const std::vector<VkSurfaceFormatKHR>&  surfaceFormats,
                            const std::vector<VkPresentModeKHR>&    surfacePresentModes);

            /// Check if surface is properly configured.
            bool isSupported() const;

            void setSize(const VkExtent2D& extent)
            {
                _configuration._extent = extent;
            }

            const Configuration& getConfiguration() const   { return _configuration; }

        private:
            Configuration _configuration;   ///< Current configuration.
    };

    using SurfaceFormatSPtr = std::shared_ptr<SurfaceFormat>;
}