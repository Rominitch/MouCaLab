/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTAnimationBones.h"
#include "LibRT/include/RTMesh.h"

#include "LibMedia/include/AnimationLoader.h"
#include "LibMedia/include/MeshLoader.h"

namespace Media
{

void MeshLoader::createMesh(RT::MeshImport& mesh) const
{
    MouCa::preCondition(Core::File::isExist( mesh.getFilename()));
    MouCa::preCondition(mesh.getDescriptor().getNbDescriptors() > 0);
    
    // Build import flag for ASSIMP
    int flag = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices;
    if(mesh.getFlag() & RT::MeshImport::SpecialExport)
        flag = aiProcess_FlipWindingOrder;
    if((mesh.getFlag() & RT::MeshImport::ComputeTangent) == RT::MeshImport::ComputeTangent )
        flag |= aiProcess_CalcTangentSpace;
    if((mesh.getFlag() & RT::MeshImport::ComputeNormal) == RT::MeshImport::ComputeNormal )
        flag |= aiProcess_GenSmoothNormals;
    const bool invertedY = (mesh.getFlag() & RT::MeshImport::InvertY) == RT::MeshImport::InvertY;

    // Create an instance of the Importer class
    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(Core::convertToU8(mesh.getFilename()), flag);

    //	unsigned int uiNode=0;
    if(pScene != nullptr) //for(unsigned int uiNode=0; uiNode < pNode->mNumMeshes; ++uiNode)
    {
        //Create new array
        size_t szVBOSize = 0;
        size_t szIBOSize = 0;
        computeVBOIBOSize(pScene, szVBOSize, szIBOSize);
        MouCa::assertion(szVBOSize > 0);
        MouCa::assertion(szIBOSize > 0);

        //Allocate buffer
        RT::BufferCPUSPtr VBOQuad = std::make_shared<RT::BufferCPU>();
        float* pVertexQuad = reinterpret_cast<float*>(VBOQuad->create(mesh.getDescriptor(), szVBOSize));

        RT::BufferCPUSPtr IBOQuad = std::make_shared<RT::BufferCPU>();
        uint32_t* pIndexQuad = reinterpret_cast<uint32_t*>(IBOQuad->create(RT::Mesh::getStandardIBOFormat(), szIBOSize));
        
        RT::Mesh::SubMeshDescriptors descriptor;
        descriptor.resize(pScene->mNumMeshes);

        //Load only the first
        RT::BoundingBox bbox;
        computeVBOIBO(pScene, pVertexQuad, pIndexQuad, mesh.getDescriptor(), descriptor, invertedY, bbox);

        // Clean empty descriptor
        auto itDescriptor = descriptor.begin();
        while( itDescriptor != descriptor.end() )
        {
            if( itDescriptor->_nbIndices != 3 )
            {
                itDescriptor = descriptor.erase(itDescriptor);
            }
            else
            {
                ++itDescriptor;
            }
        }

        mesh.getEditMesh().initialize(VBOQuad, IBOQuad, descriptor, RT::FaceOrder::CounterClockWise, RT::Topology::Triangles, bbox );
        MouCa::postCondition(mesh.getEditMesh().getNbVertices()  > 0); // DEV Issue: nothing ?
        MouCa::postCondition(mesh.getEditMesh().getNbPolygones() > 0); // DEV Issue: nothing ?
    }
}

void MeshLoader::computeVBOIBOSize(const struct aiScene* pScene, size_t& szVBOSize, size_t& szIBOSize) const
{
    for(unsigned int uiMesh = 0; uiMesh < pScene->mNumMeshes; ++uiMesh)
    {
        const struct aiMesh* pMesh = pScene->mMeshes[uiMesh];
        szVBOSize += pMesh->mNumVertices;

        for(unsigned int uiFace = 0; uiFace < pMesh->mNumFaces; ++uiFace)
        {
            const struct aiFace* pFace = &pMesh->mFaces[uiFace];
            if(pFace->mNumIndices == 3)
                ++szIBOSize;
        }
    }
}

void MeshLoader::computeVBOIBO(const struct aiScene* pScene, float* pCurrentVertex, uint32_t* pCurrentIndex, const RT::BufferDescriptor& description, std::vector<RT::Mesh::SubMeshDescriptor>& descriptors, const bool invertedY, RT::BoundingBox& bbox ) const
{
    MouCa::preCondition(pScene != nullptr);
    MouCa::preCondition(pCurrentVertex != nullptr);
    MouCa::preCondition(pCurrentIndex != nullptr);
    MouCa::preCondition(description.getNbDescriptors() > 0 && description.getComponentDescriptor(0).getComponentUsage() == RT::ComponentUsage::Vertex);

    RT::AnimationBones bones;

    const auto zero2Copy = [](const struct aiMesh* , const uint32_t , float*& pCurrentVertex)
    {
        const float zero[] = {0.0f, 0.0f};
        memcpy(pCurrentVertex, zero, sizeof(float) * 2);
        pCurrentVertex = &pCurrentVertex[2];
    };
    const auto zero3Copy = [](const struct aiMesh* , const uint32_t , float*& pCurrentVertex)
    {
        const float zero[] = {0.0f, 0.0f, 0.0f};
        memcpy(pCurrentVertex, zero, sizeof(float) * 3);
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto positionCopy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mVertices[vertexIndex].x, sizeof(float) * 3);
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto positionYInvCopy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mVertices[vertexIndex].x, sizeof(float) * 3);
        pCurrentVertex[1] = -pCurrentVertex[1];
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto normalCopy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mNormals[vertexIndex].x, sizeof(float) * 3);
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto normalYInvCopy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mNormals[vertexIndex].x, sizeof(float) * 3);
        pCurrentVertex[1] = -pCurrentVertex[1];
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto tangentCopy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mTangents[vertexIndex].x, sizeof(float) * 3);
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto texcoord0Copy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mTextureCoords[0][vertexIndex].x, sizeof(float) * 2);
        pCurrentVertex = &pCurrentVertex[2];
    };
    const auto colorCopy = [](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        memcpy(pCurrentVertex, &pMesh->mColors[0][vertexIndex].r, sizeof(float) * 3);
        pCurrentVertex = &pCurrentVertex[3];
    };
    const auto boneWeightsCopy = [&](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        auto& arrayWeights = bones._bones[vertexIndex]._weights;
        memcpy(pCurrentVertex, arrayWeights.data(), sizeof(float) * arrayWeights.size());
        pCurrentVertex = &pCurrentVertex[arrayWeights.size()];
    };
    const auto boneIDsCopy = [&](const struct aiMesh* pMesh, const uint32_t vertexIndex, float*& pCurrentVertex)
    {
        auto& arrayIDs = bones._bones[vertexIndex]._IDs;
        memcpy(pCurrentVertex, arrayIDs.data(), sizeof(int) * arrayIDs.size());
        pCurrentVertex = &pCurrentVertex[arrayIDs.size()];
    };

    uint32_t indexBase = 0;

    MouCa::assertion(descriptors.size() == pScene->mNumMeshes);
    for(uint32_t uiMesh = 0; uiMesh < pScene->mNumMeshes; ++uiMesh)
    {
        const struct aiMesh* pMesh = pScene->mMeshes[uiMesh];
        MouCa::assertion(pMesh != nullptr);

        // Need to load bones
        if( description.hasComponentUsage(RT::ComponentUsage::BonesWeights) )
        {
            AnimationLoader::loadBones(pMesh, bones);
        }

        std::vector< std::function<void(const struct aiMesh*, const uint32_t vertexIndex, float*& pCurrentVertex)> > copier;

        // Complete one time extractor
        for(size_t descriptionID=0; descriptionID < description.getNbDescriptors(); ++descriptionID)
        {
            const RT::ComponentDescriptor& descriptor = description.getComponentDescriptor(descriptionID);
            switch(descriptor.getComponentUsage())
            {
                case RT::ComponentUsage::Vertex:
                    if(pMesh->mVertices != nullptr)
                    {
                        if(!invertedY)
                            copier.emplace_back(positionCopy);
                        else 
                            copier.emplace_back(positionYInvCopy);
                    }
                    else
                        copier.emplace_back(zero3Copy);
                break;
                case RT::ComponentUsage::Normal:
                    if(pMesh->mNormals != nullptr)
                    {
                        if(!invertedY)
                            copier.emplace_back(normalCopy);
                        else
                            copier.emplace_back(normalYInvCopy);
                    }
                    else
                        copier.emplace_back(zero3Copy);
                break;
                case RT::ComponentUsage::TexCoord0:
                    if(pMesh->HasTextureCoords(0))
                        copier.emplace_back(texcoord0Copy);
                    else
                        copier.emplace_back(zero2Copy);
                break;
                case RT::ComponentUsage::Tangent:
                    if(pMesh->mTangents != nullptr)
                        copier.emplace_back(tangentCopy);
                    else
                        copier.emplace_back(zero3Copy);
                break;
                case RT::ComponentUsage::Color:
                    if(pMesh->mColors[0] != nullptr)
                        copier.emplace_back(colorCopy);
                    else
                        copier.emplace_back(zero3Copy);
                break;

                case RT::ComponentUsage::BonesWeights:
                    if( descriptor.getFormatType() == RT::Type::Float && descriptor.getNbComponents() == 4 )
                        copier.emplace_back(boneWeightsCopy);
                    else
                        MouCa::assertion(false);
                break;

                case RT::ComponentUsage::BonesIds:
                    if( descriptor.getFormatType() == RT::Type::Int && descriptor.getNbComponents() == 4 )
                        copier.emplace_back(boneIDsCopy);
                    else
                        MouCa::assertion(false);
                break;
                default:
                    MouCa::assertion(false);
            }
        }

        // Extract mesh using copier
        for(uint32_t vertexIndex = 0; vertexIndex < pMesh->mNumVertices; ++vertexIndex)
        {
            for(const auto& copy : copier)
            {
                copy(pMesh, vertexIndex, pCurrentVertex);
            }
        }

        // Compute bounding box
        bbox.expand( pMesh->mNumVertices, reinterpret_cast<RT::Point3*>(pMesh->mVertices) );
        
        //uint32_t* startIndex = pCurrentIndex;
        uint32_t countValid = 0;
        for(uint32_t uiFace = 0; uiFace < pMesh->mNumFaces; ++uiFace)
        {
            const struct aiFace* pFace = &pMesh->mFaces[uiFace];
            if(pFace->mNumIndices == 3)
            {
                pCurrentIndex[0] = pFace->mIndices[0] + indexBase;
                pCurrentIndex[1] = pFace->mIndices[1] + indexBase;
                pCurrentIndex[2] = pFace->mIndices[2] + indexBase;
                
                pCurrentIndex = &pCurrentIndex[3];
                ++countValid;
            }
            //else
                //MouCa::assertion_HEADER(false, "Number of indice is not 3.");
        }

        descriptors[uiMesh]._startIndex = indexBase;
        descriptors[uiMesh]._nbVertices = pMesh->mNumVertices;
        descriptors[uiMesh]._material   = static_cast<uint8_t>(pMesh->mMaterialIndex);
        descriptors[uiMesh]._nbIndices  = countValid * 3;

        indexBase += countValid * 3;
    }
}


}