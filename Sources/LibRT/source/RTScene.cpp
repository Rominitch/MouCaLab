/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibRT/include/RTMassiveInstance.h>
#include <LibRT/include/RTScene.h>

namespace RT
{

void Scene::release()
{
    //_root.
    _cameras.clear();
    _geometries.clear();
    _sources.clear();

    for(auto& massive : _massives)
    {
        massive->release();
    }
    _massives.clear();

    for( auto& object : _objects )
    {
        object->release();
    }
    _objects.clear();
}

}