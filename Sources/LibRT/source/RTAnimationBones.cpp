/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTAnimationBones.h"

namespace RT
{

void AnimationImporter::initialize(AnimationBonesSPtr animation)
{
    MouCa::preCondition(!isNull() && !_animation);

    _animation = animation;

    MouCa::preCondition(!isNull());
}

void AnimationImporter::release()
{
    MouCa::preCondition(!isNull());

    _filename.clear();
    _animation.reset();

    MouCa::preCondition(isNull());
}

void AnimationBones::updateAnimation(AnimatedGeometry& geometry, const uint32_t idAnimation, const double time) const
{
    auto& buffer = geometry.getEditBonesBuffer();

    const Animation& animation = _animations[idAnimation];

    // Get animation key frame
    auto itNext = std::find_if(animation.cbegin(), animation.cend(), [&](const BonesFrame& frame) {return frame._time > time; });
    auto itFrame = itNext;
    --itFrame;
    MouCa::assertion(itNext != animation.cbegin());
    if( itNext == animation.cend() )
    {
        if( !geometry.hasLoop() )
            return;
        else
            itNext = animation.cbegin();
    }
    
    // Find interpolation factor
    const double interpolate = (time - itFrame->_time) / ( itNext->_time - itFrame->_time );
    MouCa::assertion(0.0 <= interpolate && interpolate <= 1.0);

    // Parsing all nodes
    computeNodeAnimation(buffer, geometry.getBones(), Transform(), *itFrame, *itNext, interpolate);

#ifdef USE_MATRIX
    // Latest element
    buffer[buffer.size()-1]._transform = geometry.getOrientation().getLocalToWorld().convert();
#else
    // Latest element
    buffer[buffer.size() - 1]._transform = geometry.getOrientation().getLocalToWorld();
#endif
}

void AnimationBones::computeNodeAnimation(RT::BonesBuffer& buffer, const BonesHierarchy& bones, const Transform& local, const BonesFrame& start, const BonesFrame& end, const double interpolation) const
{
    uint32_t id = bones._id;
    // Compute local position
    const Transform current = Transform::slerp(start._bonesState[id], end._bonesState[id], static_cast<float>(interpolation));
    // Now global and save
    const Transform nodeTransform = local * current;
    const Transform final = nodeTransform * bones._offset;
#ifdef USE_MATRIX
    buffer[id]._transform = final.convert();
#else
    buffer[id]._transform = final;
#endif

    // Parse other children
    for(const auto& child : bones._children)
    {
        computeNodeAnimation(buffer, child, nodeTransform, start, end, interpolation);
    }
}

}