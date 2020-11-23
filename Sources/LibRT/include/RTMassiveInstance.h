/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTObject.h>

namespace RT
{
    class Mesh;
    class MeshImport;
    using MeshImportWPtr = std::weak_ptr<MeshImport>;
    using Meshes         = std::vector<MeshImportWPtr>;

    class Image;
    using ImageWPtr = std::weak_ptr<Image>;
    using Images    = std::vector<ImageWPtr>;

    //----------------------------------------------------------------------------
    /// \brief Allow to store many position/orientation of same mesh.
    ///        Example: Tree, grass, ...
    /// \code{.cpp}
    ///     
    /// \endcode
    class BasicMassiveInstance : public Object
    {
        MOUCA_NOCOPY_NOMOVE( BasicMassiveInstance );

    public:
        struct Indirect
        {
            uint32_t  _count;
            uint32_t  _indexBase;
            uint32_t  _indexCount;

            Indirect() :
                _count( 0 ),
                _indexBase( 0 ),
                _indexCount( 0 )
            {}

            Indirect( const uint32_t count, const uint32_t indexBase, const uint32_t indexCount ) :
                _count( count ),
                _indexBase( indexBase ),
                _indexCount( indexCount )
            {}
        };

        struct Instance
        {
            uint32_t  _meshID;
            RT::Point3  _position;
            RT::Point4  _quaternion;
            RT::Point3  _scale;

            Instance( const uint32_t meshID, const RT::Point3& position, const RT::Point4& quaternion, const RT::Point3& scale ) :
                _meshID( meshID ),
                _position( position ),
                _quaternion( quaternion ),
                _scale( scale )
            {}

            Instance() :
                _meshID( 0 )
            {}
        };

        /// Destructor
        ~BasicMassiveInstance() override
        {
            BT_PRE_CONDITION( isNull() );
        }

        virtual void update( const std::vector<Indirect>& indirects, const std::vector<Instance>& instances ) = 0;

        /// Release
        void release() override;

        virtual bool isNull() const
        {
            return _instances.empty() && _indirects.empty();
        }

        const std::vector<Indirect>& getIndirects() const
        {
            return _indirects;
        }

        const std::vector<Instance>& getInstances() const
        {
            return _instances;
        }

    protected:
        /// Constructor
        BasicMassiveInstance( const Type type ):
        Object( type )
        {
            BT_PRE_CONDITION( isNull() );
        }

        std::vector<Indirect>   _indirects;     ///< Indirect data.
        std::vector<Instance>   _instances;     ///< Instance data.
    };

    //----------------------------------------------------------------------------
    /// \brief Allow to store many position/orientation of same mesh.
    ///        Example: Tree, grass, ...
    /// \code{.cpp}
    ///     
    /// \endcode
    class MassiveInstance final : public BasicMassiveInstance
    {
        MOUCA_NOCOPY_NOMOVE(MassiveInstance);

        public:
            /// Constructor
            MassiveInstance():
            BasicMassiveInstance(Object::TMassive)
            {
                BT_PRE_CONDITION(isNull());
            }

            /// Destructor
            ~MassiveInstance() override
            {
                BT_PRE_CONDITION(isNull());
            }
  
            /// Release
            void release() override;

            bool isNull() const override
            {
                return _meshes.empty() && BasicMassiveInstance::isNull();
            }

            void update( const std::vector<Indirect>& indirects, const std::vector<Instance>& instances ) override;

            //------------------------------------------------------------------------
            /// \brief  Add new mesh into instance object based on mesh.
            /// 
            /// \param[in] mesh: link to mesh. Here, we don't manage resource.
            void addMesh( const MeshImportWPtr& mesh );

            //------------------------------------------------------------------------
            /// \brief  Add new image used by all meshes.
            /// 
            /// \param[in] image: link to image. Here, we don't manage resource.
            void addImage(const RT::ImageWPtr& image);

            //------------------------------------------------------------------------
            /// \brief  Get list of images.
            /// 
            /// \returns List of images.
            const Images& getImages() const
            {
                return _images;    ///< Images attached.
            }

            const Meshes& getMeshes() const
            {
                return _meshes;
            }

        private:
            Meshes  _meshes;         ///< Meshes attached.
            Images  _images;        ///< Images attached.
    };

    using MassiveInstanceSPtr = std::shared_ptr<MassiveInstance>;
}