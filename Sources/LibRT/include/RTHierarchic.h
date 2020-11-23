/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class Hierarchic;
    using HierarchicSPtr = std::shared_ptr<Hierarchic>;
    using HierarchicWPtr = std::weak_ptr<Hierarchic>;

    ///	\brief	Allow to manage tree (use for SceneGraph).
    ///         This structure are not in charge of ownership and release memory (you must KEEP outside all pointer).
    class Hierarchic : public std::enable_shared_from_this<Hierarchic>
    {
        MOUCA_NOCOPY_NOMOVE(Hierarchic);
        public:
            virtual ~Hierarchic() = default;

            void addChild(const HierarchicSPtr& pChild)
            {
                _children.insert(pChild);
                pChild->setParent(shared_from_this());
            }

            void removeChild(const HierarchicSPtr& pChild)
            {
                _children.erase(pChild);
            }

            void setParent(HierarchicSPtr parent)
            {
                _parent = parent;
            }

        protected:
            HierarchicSPtr			    _parent;

            std::set<HierarchicSPtr>	_children;

            Hierarchic() = default;
    };
}

