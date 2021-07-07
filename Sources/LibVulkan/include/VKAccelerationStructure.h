/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKAccelerationStructureGeometry.h>

namespace RT
{
    class Mesh;
}

namespace Vulkan
{
    class Device;

    class AccelerationStructure
    {
        MOUCA_NOCOPY_NOMOVE(AccelerationStructure);

        public:
            AccelerationStructure();
            ~AccelerationStructure() = default;

            void addGeometry(AccelerationStructureGeometryUPtr&& geometry);
            void setCreateInfo(const VkAccelerationStructureCreateFlagsKHR createFlags, const VkAccelerationStructureTypeKHR type);

            void initialize(const Device& device);

            void release(const Device& device);

            VkAccelerationStructureKHR getHandle() const       { return _handle; }
            uint64_t                   getDeviceAdress() const { return _deviceAddress; }

            bool isNull() const { return _handle == VK_NULL_HANDLE; }

            /// Create acceleration structure into single command
            static void createAccelerationStructure(const Device& device, std::vector<AccelerationStructureWPtr>& accelerationStructures);

        private:
            struct BuildGeometry
            {
                BuildGeometry();

                std::vector<VkAccelerationStructureBuildRangeInfoKHR>   _ranges;

                std::vector<VkAccelerationStructureGeometryKHR>         _geometriesVK;
                VkAccelerationStructureBuildSizesInfoKHR                _buildInfo;
                VkAccelerationStructureBuildGeometryInfoKHR             _geometryInfo;
            };

            void buildGeometry(const Device& device);


            Buffer                     _data;                               ///< Memory of buffer
            VkAccelerationStructureKHR _handle        = VK_NULL_HANDLE;
            uint64_t                   _deviceAddress = 0;

            VkAccelerationStructureCreateFlagsKHR   _createFlags = 0;
            VkAccelerationStructureTypeKHR          _type        = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
            
            // Geometry info
            std::vector<AccelerationStructureGeometryUPtr> _geometries;
            BuildGeometry                                  _buildGeometry;  ///< Computed data and keep for building
    };

    using AccelerationStructureSPtr = std::shared_ptr<AccelerationStructure>;
    using AccelerationStructureWPtr = std::weak_ptr<AccelerationStructure>;  
  
}