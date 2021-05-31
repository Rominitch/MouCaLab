#pragma once

#include <LibVulkan/include/VKBuffer.h>

namespace RT
{
    class Mesh;
    using MeshWPtr = std::weak_ptr<Mesh>;
}

namespace Vulkan
{
    // Forward declaration
    class AccelerationStructure;
    using AccelerationStructureWPtr = std::weak_ptr<AccelerationStructure>;

    class Device;

    class AccelerationStructureGeometry
    {
        MOUCA_NOCOPY_NOMOVE(AccelerationStructureGeometry);
        public:
            virtual ~AccelerationStructureGeometry() = default;

            void initialize(const VkGeometryFlagsKHR geometryFlags);

            const VkAccelerationStructureGeometryKHR& getGeometry() const { return _geometry; }

            uint32_t getCount() const { return _count; }

            const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& getRangeInfo() const { return _rangeInfos; }

            virtual void create(const Device& device) = 0;

        protected:
            AccelerationStructureGeometry(const VkGeometryTypeKHR type);

            VkAccelerationStructureGeometryKHR _geometry;
            uint32_t                           _count = 0;

            std::vector<VkAccelerationStructureBuildRangeInfoKHR> _rangeInfos;
    };

    using AccelerationStructureGeometryUPtr = std::unique_ptr<AccelerationStructureGeometry>;

    class AccelerationStructureGeometryInstance : public AccelerationStructureGeometry
    {
        MOUCA_NOCOPY_NOMOVE(AccelerationStructureGeometryInstance);
        public:
            struct Instance
            {
                void initialize(AccelerationStructureWPtr reference, const VkGeometryInstanceFlagsKHR flag, VkTransformMatrixKHR&& transform4x3,
                                const uint32_t instanceCustomIndex = 0, const uint32_t instanceShaderBinding = 0, const uint32_t mask = 0xFF);

                const VkAccelerationStructureInstanceKHR& compute();

                AccelerationStructureWPtr          _reference;
                VkAccelerationStructureInstanceKHR _instance;
            };

            AccelerationStructureGeometryInstance();
            ~AccelerationStructureGeometryInstance() override = default;

            void addInstance(Instance&& instance)
            {
                _instances.emplace_back(std::move(instance));
            }

            void create(const Device& device) override;
       

        private:
            std::vector<Instance> _instances;
            Buffer                _data;       ///< Memory of buffer
    };

    class AccelerationStructureGeometryTriangles : public AccelerationStructureGeometry
    {
        MOUCA_NOCOPY_NOMOVE(AccelerationStructureGeometryTriangles);
        public:
            AccelerationStructureGeometryTriangles();
            ~AccelerationStructureGeometryTriangles() override = default;

            void initialize(const RT::MeshWPtr mesh, const BufferWPtr vbo, const BufferWPtr ibo,
                            const VkGeometryFlagsKHR geometryFlags);

            void create(const Device& device) override;

        private:
            RT::MeshWPtr _mesh;
            BufferWPtr _vbo;
            BufferWPtr _ibo;
    };

}