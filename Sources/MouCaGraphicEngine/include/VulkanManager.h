/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTWindow.h>

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKSurfaceFormat.h>
#include <LibVulkan/include/VKWindowSurface.h>

namespace BT
{
    class Resource;
}

namespace RT
{
    class ImageImport;
    using ImageImportSPtr = std::shared_ptr<ImageImport>;

    class Window;
    class RenderDialog;
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;

    class ShaderFile;
    using ShaderFileWPtr = std::weak_ptr<ShaderFile>;
}

namespace XML
{
    class Parser;
    using ParserSPtr = std::shared_ptr<Parser>;
}

namespace Vulkan
{
    class ContextDevice;
    using ContextDeviceSPtr = std::shared_ptr<ContextDevice>;
    using ContextDeviceWPtr = std::weak_ptr<ContextDevice>;
    using ContextDevices    = std::vector<ContextDeviceSPtr>;

    class ContextWindow;
    using ContextWindowSPtr = std::shared_ptr<ContextWindow>;
    using ContextWindowWPtr = std::weak_ptr<ContextWindow>;
    using ContextWindows    = std::vector<ContextWindowSPtr>;

    struct PhysicalDeviceFeatures;

    class ShaderModule;
    using ShaderModuleSPtr = std::shared_ptr<ShaderModule>;
    using ShaderModuleWPtr = std::weak_ptr<ShaderModule>;
}

namespace MouCaGraphic
{
    class IEngine3DLoader;

    //---------------------------------------------------------------------------------------------
    /// \brief Allow to build and store all Vulkan items.
    class VulkanManager
    {
        MOUCA_NOCOPY_NOMOVE(VulkanManager);

        public:
            using ShaderRegistration = std::pair<Vulkan::ContextDeviceWPtr, Vulkan::ShaderModuleWPtr>;
            using ShadersDictionary  = std::map<RT::ShaderFileWPtr, ShaderRegistration, Core::WeakCompare<RT::ShaderFile>>;

            static const float          _version;
            static const Core::String   _osName;
            static const bool           _enableValidator;

            /// Constructor
            VulkanManager() = default;

            /// Destructor
            ~VulkanManager();

            //------------------------------------------------------------------------
            /// \brief  Add an existing window to be manage by rendering system.
            /// 
            /// \param[in] window: valid window.
            /// \returns Identification to retrieve Surface.
            uint32_t addRenderDialog(RT::RenderDialogWPtr window);

            /// Configure manager: create minimal environment to work.
            void initialize(const RT::ApplicationInfo& info, const std::vector<const char*>& extensions);

            /// Release all items into manager and minimal environment.
            /// \note Re-entrance authorized.
            void release();

            void execute(const uint32_t deviceID, const uint32_t sequenceID, const bool sync = true) const;

            void takeScreenshot(const Core::Path& imageFilePath, RT::ImageImportSPtr diskImage, const uint32_t contextWindowID) const;

            //Vulkan::WindowSurface& getSurface(const uint32_t id) { return _surfaces.at(id); }

        //-----------------------------------------------------------------------------------------
        //                                   Shader live programming
        //-----------------------------------------------------------------------------------------
            void registerShader(RT::ShaderFileWPtr file, const ShaderRegistration& shader);
            void afterShaderEdition(Core::Resource& shaderFile);

        //-----------------------------------------------------------------------------------------
        //                                  Environment extensions
        //-----------------------------------------------------------------------------------------
            /// Default Environment with surface support
            static std::vector<const char*> getSurfaceExtensions()
            {
                // Extension to support
                const std::vector<const char*> extensions =
                {
                    VK_KHR_SURFACE_EXTENSION_NAME,
            #if defined(VK_USE_PLATFORM_WIN32_KHR)
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
            #elif defined(VK_USE_PLATFORM_XCB_KHR)
                    VK_KHR_XCB_SURFACE_EXTENSION_NAME
            #elif defined(VK_USE_PLATFORM_XLIB_KHR)
                    VK_KHR_XLIB_SURFACE_EXTENSION_NAME
            #endif
                };
                return extensions;
            }
        //-----------------------------------------------------------------------------------------
        //                                  Device extensions
        //-----------------------------------------------------------------------------------------
            /// Default Device with surface support
            static std::vector<const char*> getDeviceExtensions()
            {
                const std::vector<const char*> deviceExtensions =
                {
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                    VK_KHR_MAINTENANCE1_EXTENSION_NAME                         // Negative Viewport
                };
                return deviceExtensions;
            }

        //-----------------------------------------------------------------------------------------
        //                                         Getter
        //-----------------------------------------------------------------------------------------
            const Vulkan::ContextDevices& getDevices() const        { return _devices; }
            const Vulkan::WindowSurfaces& getSurfaces() const       { return _surfaces; }
            const Vulkan::Environment&    getEnvironment() const    { return _environment; }
            const Vulkan::ContextWindows& getContextWindows() const { return _windows; }

        //-----------------------------------------------------------------------------------------
        //                                      Build item
        //-----------------------------------------------------------------------------------------
            Vulkan::ContextDeviceWPtr createRenderingDevice(const std::vector<const char*>& deviceExtensions, const Vulkan::PhysicalDeviceFeatures& mandatoryFeatures, Vulkan::WindowSurface& surface);

            Vulkan::ContextWindowWPtr createWindowSurface(Vulkan::ContextDeviceWPtr context, const uint32_t surfaceID, const Vulkan::SurfaceFormat::Configuration& configuration);

        //-----------------------------------------------------------------------------------------
        //                                      Writer access
        //-----------------------------------------------------------------------------------------
            Vulkan::Environment&    getEnvironment()    { return _environment; }
            Vulkan::WindowSurfaces& getSurfaces()       { return _surfaces; }
            ShadersDictionary&      getShader()         { return _shaders; }

            std::mutex&             getLocked()         { return _locked; }

        //-----------------------------------------------------------------------------------------
        //                                      Event access
        //-----------------------------------------------------------------------------------------
            Core::Signal<>&       getAfterResize()        { return _afterResize; }
            Core::Signal<>&       getAfterClose()         { return _afterClose; }
            Core::Signal<>&       getAfterShaderChanged() { return _afterShaderChanged; }

        private:
            void buildSurface(Vulkan::WindowSurface& surface) const;
            void releaseSurface(Vulkan::WindowSurface& surface);

        //-----------------------------------------------------------------------------------------
        //                                      Events
        //-----------------------------------------------------------------------------------------
            void afterClose(RT::Window* window);
            void afterResizeWindow(RT::Window* window, const RT::Array2ui& size);
            void afterStateSizeWindow(RT::Window* window, RT::Window::StateSize state);

        //-----------------------------------------------------------------------------------------
        //                                      DATA
        //-----------------------------------------------------------------------------------------
            Vulkan::Environment     _environment;       ///< [OWNERSHIP] All devices information based on computer's state.
            Vulkan::ContextDevices  _devices;           ///< [OWNERSHIP] All allocated devices.
            Vulkan::ContextWindows  _windows;           ///< [OWNERSHIP] All allocated devices.

            Vulkan::WindowSurfaces  _surfaces;          ///< [OWNERSHIP] All surface/window available.
            
            ShadersDictionary       _shaders;           ///< [LINK] Dictionary linking Resource and Shader module.
            std::mutex              _locked;

            // Event
            Core::Signal<>            _afterResize;
            Core::Signal<>            _afterClose;
            Core::Signal<>            _afterShaderChanged;
    };

}