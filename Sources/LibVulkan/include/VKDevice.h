/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class CommandBufferOld;
    class Environment;
    class Fence;    
    class Surface;

    //----------------------------------------------------------------------------
    /// \brief Allow to sort device candidate inside Device::initializeBestGPU.
    struct DeviceCandidate
    {
        // ID
        VkPhysicalDevice            deviceID;
        uint32_t                    queueFamilyID;

        // Criterion
        VkPhysicalDeviceProperties  properties;
        VkPhysicalDeviceFeatures    features;

        struct Less
        {
            bool operator()(const DeviceCandidate& ref, const DeviceCandidate& compare) const
            {
                int score = 0;
                int scoreOther = 0;

                if(ref.properties.limits.maxImageDimension2D < compare.properties.limits.maxImageDimension2D) { ++scoreOther; }
                else { ++score; }
                if(ref.properties.limits.maxImageDimension3D < compare.properties.limits.maxImageDimension3D) { ++scoreOther; }
                else { ++score; }

                return score < scoreOther;
            }
        };
    };

    //----------------------------------------------------------------------------
    /// \brief All-ow to manage Vulkan Device (VkDevice) and store physical device attached and other useful data.
    /// We can create device using initialize() based on VkPhysicalDevice ID or initializeBestGPU() based on best device.
    /// \code{.cpp}
    ///     // Initialize properly based on Real device inside computer:
    ///
    ///     Vulkan::Environment environment;
    ///     environment.initialize();
    ///     Vulkan::Device device;
    ///
    ///     device.initializeBestGPU(environment);
    /// \endcode
    class Device final
    {
        private:
            // Google test
            friend class VulkanDevice;
            FRIEND_TEST(VulkanDevice, getQueueFamiliyIndex);
            FRIEND_TEST(VulkanDevice, checkExtensions);

            VkPhysicalDevice                        _physicalDevice;        ///< Physical device ID.
            VkPhysicalDeviceFeatures                _enabledFeatures;       ///< Enabled features on physical device.
            VkPhysicalDeviceMemoryProperties        _memoryProperties;      ///< Memory properties of physical device.
            VkPhysicalDeviceProperties              _properties;            ///< Properties of physical device.

            std::vector<VkQueueFamilyProperties>    _queueFamilyProperties; ///< Queue family properties of physical device.

            VkDevice    _device;                                            ///< Device handle.
            VkQueue     _queueGraphic;                                      ///< Id of graphic queue (improve using list ?).

            VkFormat    _colorFormat;                                       ///< Best image color format of device.
            VkFormat    _depthFormat;                                       ///< Best depth buffer format of device.

            //----------------------------------------------------------------------------
            /// \brief Contains queue family indices
            struct QueueFamilyIndices
            {
                uint32_t _graphics;
                uint32_t _compute;
                uint32_t _transfer;
            } _queueFamilyIndices;

            MOUCA_NOCOPY(Device);

            void checkExtensions(const VkPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions) const;

            uint32_t getQueueFamiliyIndex(VkQueueFlagBits queueFlags) const;

            void configureDevice(const VkPhysicalDevice physicalDevice, const VkDevice deviceID, const uint32_t queueFamilyID);

        public:
            /// Constructor
            Device();
            /// Destructor
            ~Device();

            void initialize(const VkPhysicalDevice physicalDevice, const uint32_t queueFamilyID, const std::vector<const char*>& extensions = std::vector<const char*>());

            void initializeBestGPU(const Environment& environment, const std::vector<const char*>& extensions = std::vector<const char*>(), const Surface* surface=nullptr, const VkPhysicalDeviceFeatures& enabled = VkPhysicalDeviceFeatures());
            
            void release();

            //------------------------------------------------------------------------
            /// \brief  Check if device is null.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const
            {
                return _device == VK_NULL_HANDLE;
            }

            const VkPhysicalDevice& getPhysicalDevice() const
            {
                MOUCA_PRE_CONDITION(!isNull());
                return _physicalDevice;
            }

            const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const
            {
                MOUCA_PRE_CONDITION(!isNull());
                return _properties;
            }

            //------------------------------------------------------------------------
            /// \brief  Get Vulkan device handle.
            /// 
            /// \returns Vulkan device handle.
            const VkDevice& getInstance() const
            {
                MOUCA_PRE_CONDITION(!isNull());
                return _device;
            }

            uint32_t getQueueFamilyGraphicId() const
            {
                return _queueFamilyIndices._graphics;
            }
            
            uint32_t getQueueFamilyComputeId() const
            {
                return _queueFamilyIndices._compute;
            }

            uint32_t getQueueFamilyTransferId() const
            {
                return _queueFamilyIndices._transfer;
            }

            const VkQueue& getQueue() const
            {
                return _queueGraphic;
            }

            VkFormat getColorFormat() const
            {
                return _colorFormat;
            }
            
            VkFormat getDepthFormat() const
            {
                return _depthFormat;
            }

            const VkPhysicalDeviceFeatures& getEnabledFeatures() const
            {
                return _enabledFeatures;
            }

            uint32_t getMemoryType(const VkMemoryRequirements& memoryRequiered, const VkMemoryPropertyFlags properties) const;

            void waitIdle() const;

            //------------------------------------------------------------------------
            /// \brief  Check format property for current physical device.
            /// 
            /// \param[in]  format: format to check.
            /// \param[out] formatProps: format properties read for specific format.
            void readFormatProperties(const VkFormat format, VkFormatProperties& formatProps) const;
    };
}