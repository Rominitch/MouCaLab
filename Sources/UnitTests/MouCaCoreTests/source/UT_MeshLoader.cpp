#include "Dependancies.h"

#include <LibRT/include/RTMesh.h>

#include <MouCaCore/include/CoreSystem.h>
#include <MouCaCore/include/LoaderManager.h>
#include <MouCaCore/include/ResourceManager.h>

class MediaTest : public ::testing::Test
{
    protected:
        MouCaCore::CoreSystemUPtr    _core;
        MouCaCore::LoaderManager*    _loader;
        MouCaCore::ResourceManager*  _resources;

        void SetUp() override
        {
            ASSERT_NO_THROW(_core = std::make_unique<MouCaCore::CoreSystem>());
            _resources = &_core->getResourceManager();
            _loader    = &_core->getLoaderManager();
            
            // Prepare manager
            ASSERT_NO_THROW(_loader->initialize());
        }

        void TearDown() override
        {
            ASSERT_NO_THROW(_loader->release());
            ASSERT_NO_THROW(_resources->releaseResources());            
            ASSERT_NO_THROW(_core.reset());
        }
};

TEST_F(MediaTest, MeshLoaderEmpty)
{
    // Create descriptor
    RT::BufferDescriptor meshDescriptor;
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Vertex));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(2, RT::Type::Float, RT::ComponentUsage::TexCoord0));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Tangent));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Normal));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Color));

    // Load CPU data
    auto CPUMesh = _resources->openMeshImport(MouCaEnvironment::getInputPath() / L"mesh" / L"cube.obj", meshDescriptor, RT::MeshImport::ComputeTangent);
    MouCaCore::LoadingItems items =
    {
        MouCaCore::LoadingItem(CPUMesh)
    };
    ASSERT_NO_THROW(_loader->loadResources(items));

    ASSERT_FALSE(CPUMesh->isNull());

    struct Geometry
    {
        RT::Point3 vertex;
        RT::Point2 texCoord;
        RT::Point3 tangent;
        RT::Point3 normal;
        RT::Point3 color;
    };

    const std::vector<Geometry> referenceV =
    {
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},

        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},

        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},

        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},

        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},

        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f},{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }}
    };

    const std::vector<RT::Mesh::SIndex> referenceI =
    {
        {{  3,  2,  1}},
        {{  3,  1,  0}},   
        {{  7,  6,  5}},
        {{  7,  5,  4}},        
        {{ 11, 10,  9}},
        {{ 11,  9,  8}},
        {{ 15, 14, 13}},
        {{ 15, 13, 12}},
        {{ 19, 18, 17}},
        {{ 19, 17, 16}},
        {{ 23, 22, 21}},
        {{ 23, 21, 20}}
    };

    // Check Vertices
    RT::BufferCPUBaseSPtr pVBO = CPUMesh->getMesh().getVBOBuffer().lock();
    const Geometry* vertices = reinterpret_cast<Geometry*>(pVBO->lock());
    
    ASSERT_EQ(referenceV.size(), pVBO->getNbElements());
    for(size_t id = 0; id < pVBO->getNbElements(); ++id)
    {
        ASSERT_EQ(referenceV[id].vertex,   vertices[id].vertex)   << " at " << id;
        ASSERT_EQ(referenceV[id].texCoord, vertices[id].texCoord) << " at " << id;
        ASSERT_EQ(referenceV[id].tangent,  vertices[id].tangent)  << " at " << id;
        ASSERT_EQ(referenceV[id].normal,   vertices[id].normal)   << " at " << id;
        ASSERT_EQ(referenceV[id].color,    vertices[id].color)    << " at " << id;
    }

    pVBO->unlock();
    pVBO.reset();

    // Check Indices
    RT::BufferCPUBaseSPtr pIBO = CPUMesh->getMesh().getIBOBuffer().lock();
    const RT::Mesh::SIndex* indices = reinterpret_cast<RT::Mesh::SIndex*>(pIBO->lock());

    ASSERT_EQ(referenceI.size(), pIBO->getNbElements());
    for(size_t id = 0; id < pIBO->getNbElements(); ++id)
    {
        ASSERT_EQ(referenceI[id].iIndex, indices[id].iIndex) << " at " << id;
    }

    pIBO->unlock();
    pIBO.reset();
}

TEST_F(MediaTest, MeshLoaderAll)
{
    // Create descriptor
    RT::BufferDescriptor meshDescriptor;
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Vertex));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(2, RT::Type::Float, RT::ComponentUsage::TexCoord0));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Normal));

    // Load CPU data
    auto CPUMesh = _resources->openMeshImport(MouCaEnvironment::getInputPath() / L"mesh" / L"cubeComplex.obj", meshDescriptor, RT::MeshImport::DefaultImport);
    MouCaCore::LoadingItems items =
    {
        MouCaCore::LoadingItem(CPUMesh)
    };
    _loader->loadResources(items);
    ASSERT_FALSE(CPUMesh->isNull());
    
    struct Geometry
    {
        RT::Point3 vertex;
        RT::Point2 texCoord;
        RT::Point3 normal;
    };

    const std::vector<Geometry> referenceV =
    {
        { {  1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { {  1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f, -1.0f,  1.0f },{ 1.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },

        { {  1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f,  1.0f,  1.0f },{ 1.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { {  1.0f,  1.0f,  1.0f },{ 1.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },

        { {  1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f, 2.0f, 3.0f }},
        { {  1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, 2.0f, 3.0f }},
        { {  1.0f,  1.0f,  1.0f },{ 1.0f, 0.0f },{ 1.0f, 2.0f, 3.0f }},
        { {  1.0f, -1.0f,  1.0f },{ 1.0f, 1.0f },{ 1.0f, 2.0f, 3.0f }},

        { {  1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { {  1.0f,  1.0f,  1.0f },{ 0.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f,  1.0f,  1.0f },{ 1.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f, -1.0f,  1.0f },{ 1.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },

        { { -1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f,  1.0f,  1.0f },{ 0.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f,  1.0f, -1.0f },{ 1.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },

        { {  1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { {  1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f },{ 1.0f, 2.0f, 3.0f } },
        { { -1.0f,  1.0f, -1.0f },{ 1.0f, 1.0f },{ 1.0f, 2.0f, 3.0f } }
    };

    // Check Vertices
    RT::BufferCPUBaseSPtr pVBO = CPUMesh->getMesh().getVBOBuffer().lock();
    const Geometry* vertices = reinterpret_cast<Geometry*>(pVBO->lock());

    ASSERT_EQ(referenceV.size(), pVBO->getNbElements());
    for (size_t id = 0; id < pVBO->getNbElements(); ++id)
    {
        ASSERT_EQ(referenceV[id].vertex,    vertices[id].vertex)   << " at " << id;
        ASSERT_EQ(referenceV[id].texCoord,  vertices[id].texCoord) << " at " << id;
        ASSERT_EQ(referenceV[id].normal,    vertices[id].normal)   << " at " << id;
    }

    pVBO->unlock();
    pVBO.reset();
}

// DisableCodeCoverage

TEST_F(MediaTest, PERFORMANCE_MeshLoaderDAE)
{
    // Create descriptor
    RT::BufferDescriptor meshDescriptor;
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Vertex));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Color));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Tangent));
    meshDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::Float, RT::ComponentUsage::Normal));

    // Load CPU data
    auto CPUMesh = _resources->openMeshImport(MouCaEnvironment::getInputPath() / L"mesh" / L"voyager.dae", meshDescriptor);
    MouCaCore::LoadingItems items =
    {
        MouCaCore::LoadingItem(CPUMesh)
    };
    _loader->loadResources(items);
    ASSERT_FALSE(CPUMesh->isNull());
}

// EnableCodeCoverage