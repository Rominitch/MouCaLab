/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKBuffer.h>

namespace RT
{
    class Mesh;
}

namespace Vulkan
{
    class ContextDevice;

    class AccelerationBuildGeometry;
    class AccelerationStructureGeometry;
    using AccelerationStructureGeometryUPtr = std::unique_ptr<AccelerationStructureGeometry>;

    class AccelerationStructure
    {
        public:
            AccelerationStructure();

            void initialize(const ContextDevice& context, const VkAccelerationStructureCreateFlagsKHR createFlags, const VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
            void release(const ContextDevice& device);

            VkAccelerationStructureKHR getHandle() const { return _handle; }
            uint64_t                   getDeviceAdress() const { return _deviceAddress; }

            bool isNull() const { return _handle == VK_NULL_HANDLE; }

            void build(const ContextDevice& context, std::vector<AccelerationBuildGeometry>& builds);

        private:
            Buffer                     _data;                               ///< Memory of buffer
            VkAccelerationStructureKHR _handle        = VK_NULL_HANDLE;
            uint64_t                   _deviceAddress = 0;

            VkAccelerationStructureTypeKHR           _type;
            VkAccelerationStructureBuildSizesInfoKHR _buildSizeInfo;
    };

    using AccelerationStructureSPtr = std::shared_ptr<AccelerationStructure>;
    using AccelerationStructureWPtr = std::weak_ptr<AccelerationStructure>;  

    class AccelerationBuildGeometry
    {
        public:
            void initialize(const ContextDevice& context);

            void addGeometry(AccelerationStructureGeometryUPtr&& geometry);

            bool isNull() const { return _buildInfo.buildScratchSize == 0; }

            const VkAccelerationStructureBuildSizesInfoKHR& getBuildInfo() const { return _buildInfo; }

            const VkAccelerationStructureBuildGeometryInfoKHR&     getGeometryInfo() const { return _geometryInfo; }
            const std::vector<AccelerationStructureGeometryUPtr>&  getGeometries() const   { return _geometries; }
            std::vector<VkAccelerationStructureBuildRangeInfoKHR>  getRanges() const       { return _ranges; }

        private:
            std::vector<AccelerationStructureGeometryUPtr>          _geometries;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR>   _ranges;
            VkAccelerationStructureBuildSizesInfoKHR                _buildInfo;
            VkAccelerationStructureBuildGeometryInfoKHR             _geometryInfo;
    };
}