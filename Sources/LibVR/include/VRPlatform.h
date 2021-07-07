/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKSequence.h>

#include <LibVR/include/VRTrackedDevice.h>

namespace RT
{
    class Environment;
}

namespace MouCaVR
{
    //----------------------------------------------------------------------------
    /// \brief Main manager to enable VR system.
    ///
    /// \code{.cpp}
    ///   VR::Platform platform;
    ///   platform.initialize();
    ///   ...
    ///   platform.release();
    /// \endcode
    class Platform final
    {
        public:
            using VulkanHandle = void*;

            using TrackedDevices     = std::array<TrackedDevice, vr::k_unMaxTrackedDeviceCount>;
            using TrackedDevicePoses = std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount>;


            class SequenceWaitPos final : public Vulkan::Sequence
            {
                public:
                    SequenceWaitPos(Platform& platform);

                    ~SequenceWaitPos() override = default;

                    VkResult execute(const Vulkan::Device & device) override;

                    void update() override {}
                private:
                    Platform& _platform;
            };

            class SequenceSubmitVR final : public Vulkan::Sequence
            {
                public:
                    SequenceSubmitVR(const Platform& platform, const bool isLeftEye, const RT::Environment& environment, VulkanHandle deviceHandle, Platform::VulkanHandle physicalDevice, VulkanHandle queueHandle, const uint32_t queueFamilyIndex, VulkanHandle imageHandle, const uint32_t sampleCount);

                    ~SequenceSubmitVR() override = default;

                    VkResult execute(const Vulkan::Device& device) override;

                    void update() override {}
                private:
                    const Platform&           _platform;
                    vr::VRVulkanTextureData_t _vrEye;
                    vr::VRTextureBounds_t     _vrEyeBounds;
                    vr::EVREye                _eye;
            };

            Platform();
            ~Platform();

            void initialize();

            void release();

            bool isNull() const;

            //void createHeadset();
        //-----------------------------------------------------------------------------------------
        //                                      Runtime
        //-----------------------------------------------------------------------------------------
            void pollEvents();
            void pollControllerEvents();

            void updateTracked();

        //-----------------------------------------------------------------------------------------
        //                                    Information Platform
        //-----------------------------------------------------------------------------------------
            std::vector<Core::String> getInstanceExtensions() const;
            std::vector<Core::String> getPhysicalDeviceExtensions(uint64_t physicalDeviceHandle) const;

            RT::Array2ui getRenderSize() const;

            uint64_t getVulkanPhysicalDevice(const RT::Environment& environment) const;

            const TrackedDevices&   getTrackedDevices() const   { return _trackedDevices; }
            TrackedDevicePoses&     getTrackedDevicePoses()     { return _poses; }

        private:
            void getTrackedProperty(const vr::TrackedDeviceIndex_t deviceID, const vr::ETrackedDeviceProperty property, Core::String& data) const;

            vr::IVRSystem*          _system;
            vr::IVRRenderModels*    _renderModels;

            TrackedDevices      _trackedDevices;
            TrackedDevicePoses  _poses;
    };
    
}
