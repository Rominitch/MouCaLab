#include "Dependancies.h"

#include "LibRT/include/RTAnimationBones.h"
#include "LibRT/include/RTTransform.h"

#include "LibMedia/include/AnimationLoader.h"

namespace Media
{

//-------------------------------------------------------------------------------------------------
//                                      ASSIMP Private part
//-------------------------------------------------------------------------------------------------

const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string& nodeName)
{
    MOUCA_PRE_CONDITION(animation != nullptr);

    for( uint32_t i = 0; i < animation->mNumChannels; i++ )
    {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if( std::string(nodeAnim->mNodeName.data) == nodeName )
        {
            return nodeAnim;
        }
    }
    return nullptr;
}

RT::Transform toTransform(const aiMatrix4x4& matrix)
{
    aiVector3t<ai_real>    scale;
    aiVector3t<ai_real>    position;
    aiQuaterniont<ai_real> rotation;
    matrix.Decompose(scale, rotation, position);
    glm::quat quaternion(rotation.w, rotation.x, rotation.y, rotation.z);
    return RT::Transform(glm::make_vec3(&position.x), glm::make_vec3(&scale.x), quaternion);
}

void loadAnimation(RT::AnimationBones::Animation& currentAnimation, RT::AnimationBones& animationBones, RT::BonesHierarchy* node, aiAnimation* animation, const aiNode* pNode, RT::Transform parentTransform)
{
    std::string NodeName(pNode->mName.data);

    RT::Transform nodeTransform;// = parentTransform * toTransform(pNode->mTransformation);
    //glm::mat4 mat = nodeTransform.convert();
    //BT_ASSERT(mat != glm::mat4());

    const aiNodeAnim* pNodeAnim = findNodeAnim(animation, NodeName);

    RT::BonesHierarchy* current = node;
    if( pNodeAnim )
    {
        RT::BonesHierarchy& newNode = node->createChild();
        current = &newNode;
        BT_ASSERT(pNodeAnim->mNumRotationKeys == pNodeAnim->mNumPositionKeys && pNodeAnim->mNumPositionKeys == pNodeAnim->mNumScalingKeys);

        for( uint32_t id = 0; id < pNodeAnim->mNumRotationKeys; ++id )
        {
            BT_ASSERT(pNodeAnim->mRotationKeys[id].mTime == pNodeAnim->mPositionKeys[id].mTime
                   && pNodeAnim->mPositionKeys[id].mTime == pNodeAnim->mScalingKeys[id].mTime);

            nodeTransform = RT::Transform(glm::make_vec3(&pNodeAnim->mPositionKeys[id].mValue.x),
                                          glm::make_vec3(&pNodeAnim->mScalingKeys[id].mValue.x),
                                          glm::quat (pNodeAnim->mRotationKeys[id].mValue.w, 
                                                     pNodeAnim->mRotationKeys[id].mValue.x, 
                                                     pNodeAnim->mRotationKeys[id].mValue.y,
                                                     pNodeAnim->mRotationKeys[id].mValue.z));

            nodeTransform = /*parentTransform*/ nodeTransform;

            auto boneId = animationBones.boneMapping.find(NodeName);
            if( boneId != animationBones.boneMapping.end() )
            {
                current->_id     = boneId->second;
                current->_offset = animationBones.boneInfo[boneId->second].offset;

                const RT::Transform finalTransform = nodeTransform;

                auto itFrame = std::find_if(currentAnimation.begin(), currentAnimation.end(),
                                            [&](const RT::AnimationBones::BonesFrame& frame)
                                            { return frame._time == pNodeAnim->mPositionKeys[id].mTime; });
                if( itFrame == currentAnimation.end() )
                {
                    RT::AnimationBones::BonesFrame frame;
                    frame._time = pNodeAnim->mPositionKeys[id].mTime;
                    frame._bonesState.resize(animationBones.boneMapping.size());
                    frame._bonesState[boneId->second] = std::move(finalTransform);

                    currentAnimation.insert(frame);
                }
                else
                {
                    // Complexe access because std::set doesn't want modify item order: BUT SAFE because modified data is not used for indexation.
                    const RT::AnimationBones::BonesFrame& frameConst = *itFrame;
                    RT::AnimationBones::BonesFrame* frame = const_cast<RT::AnimationBones::BonesFrame*>( &frameConst );
                    frame->_bonesState[boneId->second] = std::move(finalTransform);
                }
            }
        }
    }

    // Parse children
    for( uint32_t i = 0; i < pNode->mNumChildren; i++ )
    {
        loadAnimation(currentAnimation, animationBones, current, animation, pNode->mChildren[i], nodeTransform);
    }
}

//-------------------------------------------------------------------------------------------------
//                                          MouCa part
//-------------------------------------------------------------------------------------------------
void AnimationLoader::createAnimation(RT::AnimationImporter& animationImport) const
{
    MOUCA_PRE_CONDITION(!animationImport.getFilename().empty());
    Assimp::Importer    importer;
    const aiScene*      scene = importer.ReadFile(Core::convertToU8(animationImport.getFilename()), 0);

    auto bones = std::make_shared<RT::AnimationBones>();

    // Prepare bones
    loadBones(scene->mMeshes[0], *bones.get());

    bones->_animations.resize(scene->mNumAnimations);
    // For each animation
    for( uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex )
    {
        aiAnimation* animation = scene->mAnimations[animationIndex];
        BT_ASSERT(animation != nullptr);
        
        // Extract animation
        loadAnimation(bones->_animations[animationIndex], *bones, &bones->_hierarchy, animation, scene->mRootNode, RT::Transform());
    }

    animationImport.initialize(bones);
}

void AnimationLoader::loadBones(const aiMesh* pMesh, RT::AnimationBones& bones)
{
    BT_ASSERT(pMesh->mNumBones <= RT::VertexBoneData::_maxBones);

    // Save for all vertices, the relationship between Bones/Weights
    bones._bones.resize(pMesh->mNumVertices);

    for( uint32_t i = 0; i < pMesh->mNumBones; i++ )
    {
        uint32_t index = 0;
        std::string name(pMesh->mBones[i]->mName.data);

        if( bones.boneMapping.find(name) == bones.boneMapping.end() )
        {
            // Bone not present, add new one
            index = bones.numBones;
            bones.numBones++;
            RT::BoneInfo bone;
            bones.boneInfo.push_back(bone);
            bones.boneInfo[index].offset = toTransform( pMesh->mBones[i]->mOffsetMatrix );
            bones.boneMapping[name] = index;
        }
        else
        {
            index = bones.boneMapping[name];
        }

        for( uint32_t j = 0; j < pMesh->mBones[i]->mNumWeights; j++ )
        {
            uint32_t vertexID = pMesh->mBones[i]->mWeights[j].mVertexId;
            bones._bones[vertexID].add(index, pMesh->mBones[i]->mWeights[j].mWeight);
        }
    }
}

}