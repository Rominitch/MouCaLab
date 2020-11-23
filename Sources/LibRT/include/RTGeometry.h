/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTObject.h>

namespace RT
{
    class Mesh;
    using MeshSPtr = std::shared_ptr<Mesh>;

    class MeshImport;
    using MeshImportWPtr = std::weak_ptr<MeshImport>;

    class Image;
    using ImageWPtr = std::weak_ptr<Image>;
    using Images    = std::vector<ImageWPtr>;

    class Geometry : public Object
    {
    public:
        ~Geometry() override = default;

        //------------------------------------------------------------------------
        /// \brief  Add new image into geometry.
        /// 
        /// \param[in] image: link to image. Here, we don't manage resource.
        void addImage(const RT::ImageWPtr& image);

        //------------------------------------------------------------------------
        /// \brief  Get mesh info.
        /// 
        /// \returns Mesh info.
        virtual const Mesh& getMesh() const = 0;

        const Images& getImages() const
        {
            return _images;    ///< Images attached.
        }

    protected:
        /// Constructor
        explicit Geometry(const Type type = TGeometry, const Core::String& label = u8"Default Geometry") :
        Object(type, label)
        {}

        Images          _images;    ///< Images attached.
    };

    using GeometrySPtr = std::shared_ptr<Geometry>;
    using GeometryWPtr = std::weak_ptr<Geometry>;

    class GeometryExternal : public Geometry
    {
        public:
            /// Constructor
            explicit GeometryExternal(const Core::String& label = u8"Default Geometry external"):
            Geometry(TGeometry, label)
            {}

            ~GeometryExternal() override = default;

            //------------------------------------------------------------------------
            /// \brief  Create geometry object based on mesh/bbox.
            /// 
            /// \param[in] mesh: link to mesh. Here, we don't manage resource.
            /// \param[in] bbox: Bounding box local of mesh. 
            void initialize(const MeshImportWPtr& mesh, const BoundingBox& bbox);

            //------------------------------------------------------------------------
            /// \brief  Get mesh info.
            /// 
            /// \returns Mesh info.
            const Mesh& getMesh() const override;

            const MeshImportWPtr& getImporter() const
            {
                return _mesh;
            }

            //void Export(const Core::StringOS& strFileName) const;

        protected:
            /// Inherit constructor
            GeometryExternal(const Type type, const Core::String& label):
            Geometry(type, label)
            {}


            MeshImportWPtr	_mesh;      ///< [LINK] Mesh attached.
    };

    using GeometryExternalSPtr = std::shared_ptr<GeometryExternal>;
    using GeometryExternalWPtr = std::weak_ptr<GeometryExternal>;

    class GeometryInternal : public Geometry
    {
        public:
            /// Constructor
            explicit GeometryInternal(const Core::String& label = u8"Default Geometry") :
            Geometry(TGeometry, label)
            {}

            ~GeometryInternal() override = default;

            //------------------------------------------------------------------------
            /// \brief  Create geometry object based on mesh/bbox.
            /// 
            /// \param[in] mesh: link to mesh. Here, we don't manage resource.
            /// \param[in] bbox: Bounding box local of mesh. 
            void initialize(const MeshSPtr& mesh, const BoundingBox& bbox);

            void release() override;

            //------------------------------------------------------------------------
            /// \brief  Get mesh info.
            /// 
            /// \returns Mesh info.
            const Mesh& getMesh() const override { return *_mesh.get(); }

            MeshSPtr getInternalData() { return	_mesh; }
        protected:
            MeshSPtr	_mesh;      ///< [OWNERSHIP] Mesh attached.
    };

    using GeometryInternalSPtr = std::shared_ptr<GeometryInternal>;
    using GeometryInternalWPtr = std::weak_ptr<GeometryInternal>;
}