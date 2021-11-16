/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTScene.h>

namespace RT
{
    class GeometryInternal;
    using GeometryInternalSPtr = std::shared_ptr<GeometryInternal>;
    class Window;
}

namespace GUI
{
    class Page;
    using PageSPtr = std::shared_ptr<Page>;
    using PageWPtr = std::weak_ptr<Page>;

    ///	\brief	Manage all object to represent GUI.
    ///         Each Page of GUI is saved inside with all Text, Button, ...
    ///         An additional vector is use for temporary widget (based on selection or tools tips)
    class Scene final
    {
        MOUCA_NOCOPY_NOMOVE(Scene);
        public:
            /// Constructor
            Scene();
            /// Destructor
            ~Scene() = default;

            /// \brief Initialize scene using window where GUI will be shown.
            /// \param[in] window: current window with specific scaling.
            void initialize(const RT::Window& window);
            void release();

            void addPage(PageSPtr& newPage);

            void removePage(PageSPtr& page);

            PageWPtr getActivePage() const { return _activePage; }
            void setActivePage(PageWPtr page)
            {
                MouCa::preCondition(page.expired() ||
                                 std::find_if(_pages.cbegin(), _pages.cend(), [&](const auto& currentPage) { return page.lock() == currentPage; }) != _pages.cend());
                _activePage = page;
            }

            /// \brief Convert Page data to graphical object ready for GPU transfer.
            void update();
            const RT::Scene& getGraphicalScene() const { return _scene; }

        private:
            std::vector<PageSPtr>   _pages;         ///< [OWNERSHIP] Contains all pages to show.
            PageWPtr                _activePage;    ///< Activated page from _pages list.
            float                   _globalScale;   ///<

            RT::Scene               _scene;         ///< Graphical scene

            RT::GeometryInternalSPtr _widgetQuad;

            std::vector<RT::Point3>     faceVertices;
            std::vector<RT::IBOElement> triangles;
    };
}
