#pragma once

namespace RT
{
    class Mesh;
    class BufferDescriptor;

    struct Mesh::SubMeshDescriptor;
    class MeshImport;
}

namespace Media
{
    class MeshLoader
    {
        private:
            MOUCA_NOCOPY(MeshLoader);

            void computeVBOIBOSize(const struct aiScene* pScene, size_t& szVBOSize, size_t& szIBOSize) const;

            void computeVBOIBO(const struct aiScene* pScene, float* pCurrentVertex, uint32_t* pCurrentIndex, const RT::BufferDescriptor& description, std::vector<RT::Mesh::SubMeshDescriptor>& _descriptor, const bool invertedY, RT::BoundingBox& bbox) const;

        public:
            MeshLoader()  = default;
            ~MeshLoader() = default;

            void createMesh(RT::MeshImport& mesh) const;
    };
}