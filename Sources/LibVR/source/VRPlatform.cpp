/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTEnvironment.h"

#include "LibVR/include/VRPlatform.h"

//#define VR_VERBOSE

namespace MouCaVR
{

// Define Vulkan constant without include vulkan.h
#ifndef VK_NULL_HANDLE
#   define VK_NULL_HANDLE           0
#   define VK_FORMAT_R8G8B8A8_SRGB 43
#endif

Platform::Platform():
_system(nullptr), _renderModels(nullptr)
{
    MouCa::preCondition(isNull()); // DEV Issue: Bad constructor ?
}

Platform::~Platform()
{
    MouCa::postCondition(isNull()); // DEV Issue: Missing call release() ?
}

void Platform::initialize()
{
    MouCa::preCondition(isNull()); // DEV Issue: Already initialized

    // Loading the SteamVR Runtime
    vr::EVRInitError eError = vr::VRInitError_None;
    _system = vr::VR_Init(&eError, vr::VRApplication_Scene);

    if(eError != vr::VRInitError_None)
    {
        throw Core::Exception(Core::ErrorData("VRError", "VRInitializationError"));
    }

    _renderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
    if (_renderModels == nullptr)
    {
        vr::VR_Shutdown();
        _system = nullptr;

        throw Core::Exception(Core::ErrorData("VRError", "VRRenderModelError"));
    }

    MouCa::postCondition(!isNull());
}

void Platform::release()
{
    MouCa::preCondition(!isNull()); // DEV Issue: Missing call initialize() before ?

    _renderModels = nullptr;

    vr::VR_Shutdown();
    _system       = nullptr;

    MouCa::postCondition(isNull()); // DEV Issue: Bad destruction ?
}

bool Platform::isNull() const
{
    return _system == nullptr && _renderModels == nullptr;
}

std::vector<Core::String> Platform::getInstanceExtensions() const
{
    MouCa::preCondition(!isNull());            // DEV Issue: Missing call initialize() before ?
    MouCa::preCondition(vr::VRCompositor());   // DEV Issue: Internal error ?

    Core::String extensions;

    const uint32_t nBufferSize = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);
    if (nBufferSize > 0)
    {
        extensions.resize(nBufferSize);
        vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(extensions.data(), static_cast<uint32_t>(extensions.size()));
    }

    std::vector<Core::String> listExtensions;
    boost::split(listExtensions, extensions, [](char c) {return c == ' '; });
    return listExtensions;
}

std::vector<Core::String> Platform::getPhysicalDeviceExtensions(uint64_t physicalDeviceHandle) const
{
    MouCa::preCondition(!isNull());            // DEV Issue: Missing call initialize() before ?
    MouCa::preCondition(vr::VRCompositor());   // DEV Issue: Internal error ?

    VkPhysicalDevice_T* physicalId = reinterpret_cast<VkPhysicalDevice_T*>(physicalDeviceHandle);

    Core::String extensions;

    const uint32_t nBufferSize = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalId, nullptr, 0);
    if (nBufferSize > 0)
    {
        extensions.resize(nBufferSize);
        vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalId, extensions.data(), static_cast<uint32_t>(extensions.size()));
    }

    std::vector<Core::String> listExtensions;
    boost::split(listExtensions, extensions, [](char c) {return c == ' '; });
    return listExtensions;
}

void Platform::pollEvents()
{
    MouCa::preCondition(!isNull());            // DEV Issue: Missing call initialize() before ?

    // Process SteamVR events
    vr::VREvent_t event;
    while (_system->PollNextEvent(&event, sizeof(event)))
    {
        switch (event.eventType)
        {
            case vr::VREvent_TrackedDeviceActivated:
            {
                MouCa::assertion(event.trackedDeviceIndex < _trackedDevices.size());
#ifdef VR_VERBOSE
                std::cout << "TrackDevice ON" << std::endl;
#endif
                auto& trackedDevice = _trackedDevices[event.trackedDeviceIndex];

                trackedDevice._isConnected = true;

                // Read property
                getTrackedProperty(event.trackedDeviceIndex, vr::Prop_TrackingSystemName_String, trackedDevice._name);
                getTrackedProperty(event.trackedDeviceIndex, vr::Prop_RenderModelName_String,    trackedDevice._model);
                trackedDevice._kind = _system->GetTrackedDeviceClass(event.trackedDeviceIndex);
            }
            break;
            case vr::VREvent_TrackedDeviceDeactivated:
            {
                MouCa::assertion(event.trackedDeviceIndex < _trackedDevices.size());
#ifdef VR_VERBOSE
                std::cout << "TrackDevice OFF" << std::endl;
#endif

                auto& trackedDevice = _trackedDevices[event.trackedDeviceIndex];
                trackedDevice._isConnected = false;
            }
            break;
            case vr::VREvent_TrackedDeviceUpdated:
            {
                MouCa::assertion(event.trackedDeviceIndex < _trackedDevices.size());
#ifdef VR_VERBOSE
                std::cout << "TrackDevice Update" << std::endl;
#endif

                auto& trackedDevice = _trackedDevices[event.trackedDeviceIndex];
            }
            break;

            case vr::VREvent_Quit:
            {
#ifdef VR_VERBOSE
                std::cout << "VR Quit" << std::endl;
#endif
                _system->AcknowledgeQuit_Exiting();

                return;
            }
            break;
#ifdef VR_VERBOSE
            default:

                std::cout << "VR Pool unknown" << std::endl;
#endif
        }
    }
}

void Platform::pollControllerEvents()
{
    MouCa::preCondition(!isNull());            // DEV Issue: Missing call initialize() before ?

    // Process SteamVR controller state
    for (vr::TrackedDeviceIndex_t deviceId = 0; deviceId < vr::k_unMaxTrackedDeviceCount; deviceId++)
    {
        if (_trackedDevices[deviceId]._isConnected && _system->GetControllerState(deviceId, &_trackedDevices[deviceId]._state, sizeof(_trackedDevices[deviceId]._state)))
        {
#ifdef VR_VERBOSE
            std::cout << u8"Device: "<< deviceId << std::endl;
            std::cout << u8"Button Pressed: " << _trackedDevices[deviceId]._state.ulButtonPressed << std::endl;
            std::cout << u8"Button Touch:   " << _trackedDevices[deviceId]._state.ulButtonTouched << std::endl;
            std::cout << u8"Axis:           " << _trackedDevices[deviceId]._state.rAxis->x << u8" " << _trackedDevices[deviceId]._state.rAxis->y << std::endl;
#endif
        }
    }
}

void Platform::getTrackedProperty(const vr::TrackedDeviceIndex_t deviceID, const vr::ETrackedDeviceProperty property, Core::String& data) const
{
    vr::TrackedPropertyError error = vr::TrackedProp_Success;
    const uint32_t unRequiredBufferLen = _system->GetStringTrackedDeviceProperty(deviceID, property, nullptr, 0, &error);
    if (unRequiredBufferLen != 0)
    {
        data.resize(unRequiredBufferLen);
        _system->GetStringTrackedDeviceProperty(deviceID, property, data.data(), static_cast<uint32_t>(data.size()), &error);
    }
    
    if(error != vr::TrackedProp_Success)
    {
        throw Core::Exception(Core::ErrorData("VRError", "VRTrackedPropertyError"));
    }
}

void Platform::updateTracked()
{
    // Read all poses
    std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> poses;
    vr::VRCompositor()->WaitGetPoses(poses.data(), static_cast<uint32_t>(poses.size()), nullptr, 0 );

    // Parse all tracked device
    auto itDevice = _trackedDevices.begin();
    for (auto& pose : poses)
    {
        if(pose.bPoseIsValid)
        {
            const vr::HmdMatrix34_t& matPose = pose.mDeviceToAbsoluteTracking;
            itDevice->_transform._position = glm::vec3(matPose.m[0][3], matPose.m[1][3], matPose.m[2][3]);
            const glm::mat3 rotMatrix(matPose.m[0][0], matPose.m[1][0], matPose.m[2][0],
                                      matPose.m[0][1], matPose.m[1][1], matPose.m[2][1],
                                      matPose.m[0][2], matPose.m[1][2], matPose.m[2][2]);
            itDevice->_transform._rotation = glm::quat_cast(rotMatrix);
#ifdef VR_VERBOSE
            //std::cout << "Position: " << itDevice->_transform._position.x << itDevice->_transform._position.y << itDevice->_transform._position.z << std::endl;
#endif
        }

        ++itDevice;
    }
    MouCa::postCondition(itDevice == _trackedDevices.end());
}

RT::Array2ui Platform::getRenderSize() const
{
    MouCa::preCondition(!isNull());
    
    RT::Array2ui resolution;
    _system->GetRecommendedRenderTargetSize( &resolution.x, &resolution.y );
    return resolution;
}

uint64_t Platform::getVulkanPhysicalDevice(const RT::Environment& environment) const
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!environment.isNull());

    VkInstance_T* vkInstance = const_cast<VkInstance_T*>(reinterpret_cast<const VkInstance_T*>(environment.getGenericInstance()));
    
    // Query OpenVR for the physical device to use
    uint64_t pHMDPhysicalDevice = 0;
    _system->GetOutputDevice(&pHMDPhysicalDevice, vr::TextureType_Vulkan, vkInstance);
    return pHMDPhysicalDevice;
}

Platform::SequenceWaitPos::SequenceWaitPos(Platform& platform):
_platform(platform)
{}

VkResult Platform::SequenceWaitPos::execute(const Vulkan::Device&)
{
    auto& poses = _platform.getTrackedDevicePoses();
    vr::VRCompositor()->WaitGetPoses(poses.data(), static_cast<uint32_t>(poses.size()), nullptr, 0);
    return (VkResult)0;
}

Platform::SequenceSubmitVR::SequenceSubmitVR(const Platform& platform, const bool isLeftEye, const RT::Environment& environment, Platform::VulkanHandle deviceHandle, Platform::VulkanHandle physicalDevice, Platform::VulkanHandle queueHandle, const uint32_t queueFamilyIndex, VulkanHandle imageHandle, const uint32_t sampleCount):
_platform(platform), _eye(isLeftEye ? vr::Eye_Left : vr::Eye_Right),
_vrEye({ 0, nullptr, nullptr, nullptr, nullptr, 0, 0, 0, 0, 0 }),
_vrEyeBounds({ 0.0f, 0.0f, 1.0f, 1.0f })
{
    const auto size = _platform.getRenderSize();
    _vrEye =
    {
        reinterpret_cast<uint64_t>(imageHandle),
        reinterpret_cast<VkDevice_T*>(deviceHandle),
        reinterpret_cast<VkPhysicalDevice_T*>(physicalDevice),
        reinterpret_cast<VkInstance_T*>(const_cast<void*>(environment.getGenericInstance())),
        reinterpret_cast<VkQueue_T*>(queueHandle),
        queueFamilyIndex,
        size[0], size[1],
        VK_FORMAT_R8G8B8A8_SRGB, sampleCount
    };
}

VkResult Platform::SequenceSubmitVR::execute(const Vulkan::Device&)
{
    MouCa::preCondition(vr::VRCompositor() != nullptr);
    MouCa::preCondition(_vrEye.m_nImage != 0);

    const vr::Texture_t texture
    {
        &_vrEye,
        vr::TextureType_Vulkan,
        vr::ColorSpace_Auto
    };

    vr::VRCompositor()->Submit(_eye, &texture, &_vrEyeBounds);
    return (VkResult)0;
}

}