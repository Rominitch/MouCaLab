/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class PipelineCache
    {
        private:
            VkPipelineCache _pipelineCache; ///< Vulkan Instance of pipeline cache.

            MOUCA_NOCOPY(PipelineCache);

        public:
            PipelineCache();
            ~PipelineCache();

            void initialize(const Device& device);

            void release(const Device& device);

            bool isNull() const
            {
                return _pipelineCache == VK_NULL_HANDLE;
            }

            const VkPipelineCache& getInstance() const
            {
                return _pipelineCache;
            }
    };

}