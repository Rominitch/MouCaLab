/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibGLFW/include/GLFWHardware.h>
#include <LibGLFW/include/GLFWPlatform.h>

#include <LibVulkan/include/VKEnvironment.h>

#include <LibVR/include/VRPlatform.h>

#include <MouCaGraphicEngine/include/VulkanManager.h>

namespace MouCaGraphic
{
    class GraphicConfiguration;

    //----------------------------------------------------------------------------
    /// \brief Main 3D engine which allows to create platform object (window) and create 3D objects based on LibRT descriptor (real 3D technology is hide to final user).
    class GraphicEngine final
    {
        public:
            GraphicEngine() = default;
            ~GraphicEngine() = default;

            void initialize();
            void release();

            void loopEvent();

            void getDefaultConfiguration(GraphicConfiguration& configuration) const;

            const RT::Monitors& getMonitors() const;

        //-----------------------------------------------------------------------------------------
        //                                  DLL getter
        //-----------------------------------------------------------------------------------------
            GLFW::Platform& getRTPlatform()
            {
                return _platform;
            }

            MouCaVR::Platform& getVRPlatform()
            {
                return _vrPlatform;
            }

            GLFW::Platform& getPlatform()
            {
                return _platform;
            }

            GLFW::Hardware& getHardware()
            {
                _hardware;
            }

        private:
            GLFW::Platform          _platform;
            GLFW::Hardware          _hardware;

            MouCaVR::Platform       _vrPlatform;
            VulkanManager           _3DManager;
    };

}