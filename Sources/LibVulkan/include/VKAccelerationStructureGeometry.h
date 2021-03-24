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
    class ContextDevice;

    class AccelerationStructureGeometry
    {
        public:
            void initialize(const VkGeometryFlagsKHR geometryFlags);

            const VkAccelerationStructureGeometryKHR& getGeometry() const { return _geometry; }

            uint32_t getCount() const { return _count; }

            const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& getRangeInfo() const { return _rangeInfos; }

        protected:
            AccelerationStructureGeometry(const VkGeometryTypeKHR type);

            VkAccelerationStructureGeometryKHR _geometry;
            uint32_t                           _count = 0;

            std::vector<VkAccelerationStructureBuildRangeInfoKHR> _rangeInfos;
    };

    class AccelerationStructureGeometryInstance : public AccelerationStructureGeometry
    {
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

            void addInstance(Instance&& instance)
            {
                _instances.emplace_back(std::move(instance));
            }

            void create(const ContextDevice& context);
       

        private:
            std::vector<Instance> _instances;
            Buffer                _data;       ///< Memory of buffer
    };

    class AccelerationStructureGeometryTriangles : public AccelerationStructureGeometry
    {
        public:
            AccelerationStructureGeometryTriangles();

            void initialize(const RT::Mesh& mesh, const BufferWPtr vbo, const BufferWPtr ibo,
                            const VkGeometryFlagsKHR geometryFlags);

            void create(const ContextDevice& context);

        private:
            const RT::Mesh*  _mesh;
            BufferWPtr _vbo;
            BufferWPtr _ibo;
    };

}