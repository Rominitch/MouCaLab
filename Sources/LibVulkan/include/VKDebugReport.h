/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class DebugReport
    {
        private:
            VkDebugReportCallbackEXT _callback;

            MOUCA_NOCOPY(DebugReport);

            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

        public:
            DebugReport();
            ~DebugReport();

            void initialize(const VkInstance& vulkanInstance);
            void release(const VkInstance& vulkanInstance);

            bool isNull() const
            {
                return (_callback == VK_NULL_HANDLE);
            }

            static const char* getExtension()
            {
                return VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }

            static const std::vector<const char*> getLayers()
            {
                const std::vector<const char*> validationLayers =
                {
                    "VK_LAYER_LUNARG_standard_validation"
                };
                return validationLayers;
            }

            static void setup(VkDevice device);
    };    
}