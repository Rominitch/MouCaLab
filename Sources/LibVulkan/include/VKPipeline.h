#pragma once

namespace Vulkan
{

    class Pipeline
    {
        MOUCA_NOCOPY(Pipeline);
        public:
            virtual ~Pipeline()
            {
                MouCa::postCondition(isNull());
            }

            bool isNull() const
            {
                return _pipeline == VK_NULL_HANDLE;
            }

            const VkPipeline& getInstance() const
            {
                return _pipeline;
            }

        protected:
            Pipeline() = default;

            VkPipeline _pipeline = VK_NULL_HANDLE;
    };
}
