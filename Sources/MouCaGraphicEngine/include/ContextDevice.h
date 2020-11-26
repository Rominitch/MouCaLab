#pragma once

#include <LibVulkan/include/VKCommandPool.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKPipelineCache.h>

#include <MouCa3DVulkanEngine/include/ContextWindow.h>

namespace MouCa3DEngine
{
    //----------------------------------------------------------------------------
    /// \brief Manage a Vulkan device: create all objects which need a device to live.
    /// \author Romain MOURARET
    class ContextDevice final
    {
        BT_NOCOPY(ContextDevice);

        public:
            /// Constructor
            ContextDevice() = default;

            /// Destructor
            ~ContextDevice();

            void initialize(const Vulkan::Environment& environment, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& enabled, const Vulkan::Surface* surface = nullptr);
            
            void release();

            //------------------------------------------------------------------------
            /// \brief  Check if current device is not initialized.
            /// 
            /// \returns True if each component is null, otherwise false.
            bool isNull() const
            {
                return _device.isNull() && _commandPool.isNull() && _pipelineCache.isNull();
            }

            //------------------------------------------------------------------------
            /// \brief  Return current device.
            const Vulkan::Device& getDevice() const { return _device; }

        private:
            Vulkan::Device              _device;            ///< Vulkan Device data.
            Vulkan::CommandPool         _commandPool;       ///< Command pool.
            Vulkan::PipelineCache       _pipelineCache;     ///< Pipeline cache

            std::vector<ContextWindow>  _contextSurfaces;   ///< 
    };

    using ContextDeviceSPtr = std::shared_ptr<ContextDevice>;
    using ContextDeviceWPtr = std::weak_ptr<ContextDevice>;
    using ContextDevices    = std::vector<ContextDeviceSPtr>;
}