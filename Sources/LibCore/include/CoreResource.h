/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreString.h>

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Allow to define a file resource properties.
    /// Allow to define a file resource properties. Generally, resource item must inherit of this class.
    /// For example, a shader program where all modification need automatically a reload.
    /// \code{.cpp}
    ///     Resource myShader(myfile, Resource::AutoRefresh);
    /// \endcode
    class Resource
    {
        MOUCA_NOCOPY_NOMOVE( Resource );

        public:
            enum class Mode : uint8_t
            {
                AutoRefresh	=	0x01,	///< Reload resource when someone change it

                Critical	=	0x10,	///< If resource is critical for program

                Temporary	=	0x20,	///< If resource is allocate temporary during program execution

                // Tools
                NoRefresh   =   0x00,	///< Load resource without track it (don't test as MODE)
                AutoRefreshCritical = Critical | AutoRefresh
            };

            //------------------------------------------------------------------------
            /// \brief  Constructor
            /// 
            /// \param[in] filename: where is the file resource on HD.
            /// \param[in] eMode: how need to manage this resource.
            Resource(const Path& filename= Path(), const Mode eMode = Mode::NoRefresh):
            _eMode(eMode), _filename(filename)
            {}

            /// Destructor
            virtual ~Resource() = default;

            void setFileInfo(const Path& filename, const Mode eMode = Mode::NoRefresh)
            {
                MOUCA_PRE_CONDITION(isNull()); //DEV Issue: Need unload resource

                _filename = filename;
                _eMode    = eMode;

                MOUCA_POST_CONDITION(!isNull()); //DEV Issue: Don't change resource
            }

            virtual void release() = 0;

            //------------------------------------------------------------------------
            /// \brief  Check if resource is not loaded and have no link to file data.
            /// 
            /// \returns True if not loaded and no link, otherwise false.
            virtual bool isNull() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Check if resource is loaded.
            /// 
            /// \returns True if loaded, otherwise false.
            virtual bool isLoaded() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Check if current resource is in specific mode.
            /// 
            /// \param[in]  eMode: mode to check
            /// \returns True if mode is enabled, false otherwise.
            bool isMode(const Mode eMode) const
            {
                return static_cast<Resource::Mode>(static_cast<uint8_t>(_eMode) & static_cast<uint8_t>(eMode))
                       == eMode;
            }

            //------------------------------------------------------------------------
            /// \brief  Get current filename of resource.
            /// 
            /// \returns Current filename of resource
            const Path& getFilename() const
            {
                return _filename;
            }

            //------------------------------------------------------------------------
            /// \brief  Get current filename for tracking.
            /// 
            /// \returns Current filename for tracking.
            virtual const Path& getTrackedFilename() const
            {
                return _filename;
            }

            bool operator< (const Resource& compare) const
            {
                return this < &compare;
            }

        protected:
            Mode    _eMode;			///< Mode of resource
            Path    _filename;		///< Path to resource
    };

    using ResourceSPtr = std::shared_ptr<Resource>;
    using ResourceWPtr = std::weak_ptr<Resource>;
}
