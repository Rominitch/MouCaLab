/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreResource.h>

#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTTransform.h>

//#define USE_MATRIX

namespace RT
{
    class AnimatedGeometry;

    class VertexBoneData
    {
        MOUCA_NOCOPY(VertexBoneData);
        public:
            static const uint64_t _maxBones = 64;
            static const uint64_t _maxBonesPerVertex = 4; ///< Maximum number of bones per vertex

            uint8_t _position;
            std::array<uint32_t, _maxBonesPerVertex> _IDs;
            std::array<float,      _maxBonesPerVertex> _weights;

            VertexBoneData():
            _position(0), _IDs({ 0,0,0,0 }), _weights({ 0.0f, 0.0f, 0.0f, 0.0f })
            {}

            VertexBoneData(VertexBoneData&&) = default;

            // Add bone weighting to vertex info
            void add(uint32_t boneID, float weight)
            {
                if( _position < _maxBonesPerVertex )
                {
                    _IDs[_position] = boneID;
                    _weights[_position] = weight;
                }
                ++_position;
            }
    };

    // Stores information on a single bone
    struct BoneInfo
    {
        Transform offset;
        BoneInfo() = default;
    };

#ifdef USE_MATRIX
    /// Buffer data for shader
    struct UBOBoneTransform
    {
        glm::mat4 _transform = glm::mat4(1.0f);
    };
    using BonesBuffer = std::array<UBOBoneTransform, 1 + VertexBoneData::_maxBones>;
#else
    /// Buffer data for shader
    struct UBOBoneTransform
    {
        Transform _transform;
    };
    using BonesBuffer = std::array<UBOBoneTransform, 1 + VertexBoneData::_maxBones>;
#endif

    struct BonesHierarchy
    {
        MOUCA_NOCOPY(BonesHierarchy);

        uint32_t                  _id;
        Transform                   _offset;
        Transform                   _current;
        std::vector<BonesHierarchy> _children;

        /// Constructor
        BonesHierarchy(const Transform& transform = Transform()) :
        _id(0), _current(transform)
        {}

        BonesHierarchy(BonesHierarchy&&) = default;

        void addChild(BonesHierarchy&& child)
        {
            _children.emplace_back(std::move(child));
        }

        BonesHierarchy& createChild()
        {
            _children.emplace_back(BonesHierarchy());
            return *_children.rbegin();
        }
    };

    class AnimationBones
    {
    public:
        // Bone related stuff Maps bone name with index
        std::map<std::string, uint32_t> boneMapping;
        // Bone details
        std::vector<BoneInfo> boneInfo;
        // Number of bones present
        uint32_t numBones = 0;
        // Per-vertex bone info
        std::vector<VertexBoneData> _bones;
        
        BonesHierarchy _hierarchy;

        struct BonesFrame
        {
            double                  _time;          ///< Time position of frame.
            std::vector<Transform>  _bonesState;    ///< States of all bones.

            //------------------------------------------------------------------------
            /// \brief  Allow to order frame inside container.
            /// 
            /// \param[in]   other: element to compare
            /// \returns True if lower than other element.
            bool operator<(const BonesFrame& other) const
            {
                return _time < other._time;
            }

            bool getBeforeFrame(double time) const
            {
                return _time <= time;
            }

            bool operator() (const BonesFrame& lhs, const BonesFrame& rhs) const
            {
                return lhs < rhs;
            }
        };

        using Animation = std::set<BonesFrame, BonesFrame>; /// Animation: all frames 
        
        // Modifier for the animation 
        float animationSpeed = 0.75f;
        std::vector<Animation> _animations;

        void computeNodeAnimation(RT::BonesBuffer& buffer, const BonesHierarchy& bones, const Transform& local, const BonesFrame& start, const BonesFrame& end, const double interpolation) const;

        void updateAnimation(AnimatedGeometry& geometry, const uint32_t idAnimation, const double time) const;
    };

    class AnimationBones;
    using AnimationBonesSPtr = std::shared_ptr<AnimationBones>;

    //----------------------------------------------------------------------------
    /// \brief System to extract mesh from file.
    /// 
    class AnimationImporter final : public Core::Resource
    {
        MOUCA_NOCOPY_NOMOVE(AnimationImporter);
        public:
            /// Constructor
            AnimationImporter():
            Core::Resource()
            {
                MOUCA_PRE_CONDITION(isNull());
            }

            /// Destructor
            ~AnimationImporter()
            {
                MOUCA_POST_CONDITION(isNull());
            }

            //------------------------------------------------------------------------
            /// \brief  Release resource after use it.
            /// 
            void initialize(AnimationBonesSPtr animation);

            //------------------------------------------------------------------------
            /// \brief  Release resource after use it.
            /// 
            void release() override;

            //------------------------------------------------------------------------
            /// \brief  Check if current resource is clean (no file attached, no data).
            /// 
            /// \returns True if null, otherwise false.
            bool isNull() const override
            {
                return _filename.empty() && !isLoaded();
            }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is loaded.
            /// 
            /// \returns True if loaded, otherwise false.
            bool isLoaded() const override
            {
                return _animation != nullptr;
            }

            const AnimationBones& getAnimation() const
            {
                return *_animation;
            }

        private:
            AnimationBonesSPtr  _animation;    ///< [OWNERSHIP] Animations into file.
    };

    class AnimatedGeometry final : public GeometryExternal
    {
        MOUCA_NOCOPY_NOMOVE(AnimatedGeometry);

        public:
            AnimatedGeometry(const Core::String& label = "Default animated geometry") :
            GeometryExternal(TAnimatedGeometry, label), _loop(false)
            {}

            const BonesBuffer&      getBonesBuffer() const  { return _buffer; }
            BonesBuffer&            getEditBonesBuffer()    { return _buffer; }

            const BonesHierarchy&   getBones() const  { return *_bones; }

            void setBones(const BonesHierarchy* bones) { _bones = bones; }

            void setLoop(bool loop)         { _loop = loop; }

            bool hasLoop() const            { return _loop; }

        private:
            BonesBuffer           _buffer;        ///< Current position data.
            const BonesHierarchy* _bones;
            bool                  _loop;
    };

    using AnimatedGeometrySPtr = std::shared_ptr<AnimatedGeometry>;
}