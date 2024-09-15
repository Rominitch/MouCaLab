/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTObject.h>

namespace RT
{
    class DataEnvironment;
    class ContextExecution;

    class Object;
    using ObjectSPtr = std::shared_ptr<Object>;

    class Geometry;
    using GeometrySPtr = std::shared_ptr<Geometry>;
    
    class Light;
    using LightSPtr = std::shared_ptr<Light>;

    class Camera;
    using CameraSPtr = std::shared_ptr<Camera>;

    class MassiveInstance;
    using MassiveInstanceSPtr = std::shared_ptr<MassiveInstance>;

    // Declare array
    using GeometryArray = std::vector<GeometrySPtr>;
    using LightArray    = std::vector<LightSPtr>;
    using CameraArray   = std::vector<CameraSPtr>;
    using MassiveArray  = std::vector<MassiveInstanceSPtr>;
    using ObjectArray   = std::vector<ObjectSPtr>;

    //----------------------------------------------------------------------------
    /// \brief Scene descriptor.
    class Scene
    {
        MOUCA_NOCOPY_NOMOVE(Scene);

        public:
            Scene():
            _root(Object::TObject, "SceneRoot")
            {
                MouCa::preCondition( isNull() );
            }

            ~Scene()
            {
                MouCa::postCondition( isNull() );  ///DEV Issue: Missing calling release()
            }

            void release();
            
        //---------------------------------------------------------------
        //					Item management methods
        //---------------------------------------------------------------
            void addGeometry(GeometrySPtr geometry)
            {
                MouCa::preCondition(geometry.get() != nullptr);
                _geometries.emplace_back(geometry);
            }

            void addObjectRenderable( const ObjectSPtr& object )
            {
                MouCa::preCondition( object.get() != nullptr );
                _objects.emplace_back( object );
            }

            void addLight(const LightSPtr& light)
            {
                MouCa::preCondition(light.get() != nullptr);
                _sources.emplace_back(light);
            }

            void addCamera(const CameraSPtr& camera)
            {
                MouCa::preCondition(camera.get() != nullptr);
                _cameras.emplace_back(camera);
            }

            void addMassive(const MassiveInstanceSPtr& massive)
            {
                MouCa::preCondition(massive.get() != nullptr);
                _massives.emplace_back(massive);
            }

            const CameraArray& getCameras() const
            {
                return _cameras;
            }

            const GeometryArray& getGeometries() const
            {
                return _geometries;
            }

            const LightArray& getLights() const
            {
                return _sources;
            }

            const Object& getRoot() const
            {
                return _root;
            }

            const MassiveArray& getMassiveInstances() const
            {
                return _massives;
            }

            const ObjectArray& getObjects() const
            {
                return _objects;
            }

            bool isNull() const
            {
                return _cameras.empty() && _geometries.empty() && _sources.empty()
                    && _massives.empty() && _objects.empty();
            }

        //---------------------------------------------------------------
        //					Item management methods
        //---------------------------------------------------------------
                    
        protected:
            // List of all item inside scene
            CameraArray	    _cameras;       ///< List of cameras.
            GeometryArray	_geometries;    ///< List of geometries.
            LightArray		_sources;       ///< List of sources.
            MassiveArray    _massives;      ///< List of Massive instances.
            ObjectArray     _objects;       ///< List of custom object renderable.

            // Main root
            Object          _root;          ///< Main node of scene graph (List and node must be in relation).
    };

    using SceneSPtr = std::shared_ptr<Scene>;
}