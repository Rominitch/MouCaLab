/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTEnvironment.h>

#include <LibVulkan/include/VKDebugReport.h>

namespace RT
{
    struct ApplicationInfo;
};

namespace Vulkan
{
    using PhysicalDevices = std::vector<VkPhysicalDevice>;

    class Environment : public RT::Environment
    {
    public:
        Environment();
        ~Environment() override;

        bool isNull() const override
        {
            return _instance == VK_NULL_HANDLE;
        }

        void initialize(const RT::ApplicationInfo& info, const std::vector<const char*>& extensions = std::vector<const char*>());

        void release();

        ///	Get a valid local instance (must call initialize before).
        ///	\return	VkInstance of environment.
        const VkInstance& getInstance() const
        {
            MouCa::assertion(!isNull());
            return _instance;
        }

        const void* getGenericInstance() const override
        {
            MouCa::assertion(!isNull());
            return reinterpret_cast<void*>(_instance);
        }

        const PhysicalDevices& getGraphicsPhysicalDevices() const
        {
            return _graphicsDevices;
        }

#ifdef VULKAN_DEBUG
        void setDeviceInReport(const VkDevice device)
        {
            _report.setup(device);
        }
#endif

    private:
        VkInstance		            _instance;          ///< Vulkan Instance.
        PhysicalDevices             _rejectDevices;     ///< Physical device which not support Vulkan;
        PhysicalDevices             _graphicsDevices;   ///< Physical device specialized for Graphic;
        PhysicalDevices             _computeDevices;    ///< Physical device specialized for Compute (no graphic);

#ifdef VULKAN_DEBUG
        DebugReport         _report;
#endif
        void checkExtensions(const std::vector<const char*>& extensions) const;
    };

}