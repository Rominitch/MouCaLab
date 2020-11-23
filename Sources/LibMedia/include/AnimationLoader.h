#pragma once

namespace RT
{
    class AnimationBones;
}

struct aiMesh;

namespace Media
{

    class AnimationLoader
    {
        private:
            MOUCA_NOCOPY(AnimationLoader);

        public:
            AnimationLoader() = default;
            ~AnimationLoader() = default;

            static void loadBones(const aiMesh* pMesh, RT::AnimationBones& bones);

            void createAnimation(RT::AnimationImporter& animation) const;
    };
}