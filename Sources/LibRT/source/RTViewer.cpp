/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTCameraManipulator.h"
#include "LibRT/include/RTEventManager.h"
#include "LibRT/include/RTScene.h"
#include "LibRT/include/RTViewer.h"

namespace RT
{

bool Viewer::isNull() const
{
    return _scene == NULL
        && _renderer == NULL
        && _cameraManipulator == NULL;
}

void Viewer::setRenderer(const RT::RendererSPtr& renderer)
{
    _renderer = renderer;
}

void Viewer::setCamera(std::shared_ptr<Camera>& camera)
{
    _camera = camera;
}

void Viewer::setCameraManipulator(std::shared_ptr<CameraManipulator>& cameraManipulator)
{
    _cameraManipulator = cameraManipulator;
}

}