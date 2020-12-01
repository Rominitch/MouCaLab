#include "Dependencies.h"

#include "LibRT/include/RTBufferDescriptor.h"
#include "LibRT/include/RTBufferCPU.h"
#include "LibRT/include/RTGeometry.h"
#include "LibRT/include/RTMesh.h"
#include "LibRT/include/RTWindow.h"

#include "LibGUI/include/GUIPage.h"
#include "LibGUI/include/GUIScene.h"
#include "LibGUI/include/GUIWidget.h"

namespace GUI
{

Scene::Scene() :
_globalScale(1.0f)
{}

void Scene::initialize(const RT::Window& window)
{
    _globalScale = window.getPixelScaling().x;

    _widgetQuad = std::make_shared<RT::GeometryInternal>();
    //_scene.addGeometry(_widgetQuad);
}

void Scene::release()
{
    _pages.clear();

    _widgetQuad->release();

    _scene.release();
}

void Scene::addPage(PageSPtr& newPage)
{
    MOUCA_ASSERT(std::find(_pages.cbegin(), _pages.cend(), newPage) == _pages.cend());  // DEV Issue: already in scene ?
    _pages.push_back(newPage);
}

void Scene::removePage(PageSPtr& page)
{
    MOUCA_ASSERT(std::find(_pages.cbegin(), _pages.cend(), page) != _pages.cend());  // DEV Issue: not in scene
    _pages.erase(std::find(_pages.cbegin(), _pages.cend(), page));

    // Disable active page
    if (_activePage.lock() == page)
        _activePage.reset();
}

void Scene::update()
{
    if (!_activePage.expired())
    {
        const RT::BoundingBox bbox(RT::Point3(-1.0f,-1.0f,-1.0f), RT::Point3(1.0f,1.0f,1.0f));
        uint32_t  idTriangle = 0;

        RT::Mesh::SubMeshDescriptors subDescriptors;

        size_t startId    = 0;
        size_t nbBeforeVertices = 0;
        auto page = _activePage.lock();
        for(const auto& widget : page->getWidgets())
        {
            const Widget2D* widget2D = dynamic_cast<Widget2D*>(widget.get());
            if (widget2D != nullptr)
            {
                // Factor by anchor
                const std::array<float, 5> directions = {0.0f, 1.0f, -0.5f, 0.0f, -1.0f};

                // Size modulate by anchor
                const int dirH = (widget2D->getAnchor() & GUI::Widget2D::MaskHorizontal);
                const int dirV = (widget2D->getAnchor() & GUI::Widget2D::MaskVertical) >> 4;
                MOUCA_ASSERT(0 <= dirH && dirH < directions.size()
                       && 0 <= dirV && dirV < directions.size());
                const RT::Point2& size = widget2D->getSize()
                                       * RT::Point2(directions[dirH], directions[dirV]);
                // Build quads VBO/IBO
                faceVertices.push_back(widget2D->getPosition());
                faceVertices.push_back(widget2D->getPosition() + RT::Point3(0.0f, size.y, 0.0f) );
                faceVertices.push_back(widget2D->getPosition() + RT::Point3(size, 0.0f));
                faceVertices.push_back(widget2D->getPosition() + RT::Point3(size.x, 0.0f, 0.0f) );

                triangles.push_back({ idTriangle, idTriangle + 1, idTriangle + 2 });
                triangles.push_back({ idTriangle, idTriangle + 2, idTriangle + 3 });
                // Next Id
                idTriangle += 4;
                
                // Make instance
                subDescriptors.push_back({ faceVertices.size()-nbBeforeVertices, startId, (triangles.size() * 3)-startId, 0 });                
                startId += triangles.size() * 3;
                nbBeforeVertices = faceVertices.size();
            }
        }

        // Allocate buffer
        const std::vector<RT::ComponentDescriptor> descriptors =
        {
            { 3, RT::Type::Float, RT::ComponentUsage::Vertex  },
        };

        RT::BufferDescriptor bufferDescriptor;
        bufferDescriptor.initialize(descriptors);

        RT::BufferLinkedCPU* linkedVertices = new RT::BufferLinkedCPU();
        linkedVertices->create(bufferDescriptor, faceVertices.size(), faceVertices.data());
        std::shared_ptr<RT::BufferCPUBase> vbo(linkedVertices);
        RT::BufferLinkedCPU* linkedIBO = new RT::BufferLinkedCPU();
        linkedIBO->create(RT::Mesh::getStandardIBOFormat(), triangles.size(), triangles.data());
        std::shared_ptr<RT::BufferCPUBase> ibo(linkedIBO);

        auto mesh = std::make_shared<RT::Mesh>();
        mesh->initialize(vbo, ibo, subDescriptors);

        _widgetQuad->release();
        _widgetQuad->initialize(mesh, bbox);
        _widgetQuad->setState(RT::Object::VisibilityAlways, RT::Object::VisibilityMask);

        _scene.addGeometry(_widgetQuad);
    }
    else
    {
        _widgetQuad->setState(RT::Object::Disabled, RT::Object::VisibilityMask);
    }
}

}