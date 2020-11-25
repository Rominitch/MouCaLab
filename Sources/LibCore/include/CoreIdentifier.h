/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreString.h>

namespace Core
{
    /// Provide a Global ID for entire application + naming.
    class Identifier
    {
        MOUCA_NOCOPY_NOMOVE(Identifier);

        public:
            /// Constructor
            Identifier() : _gid(_latestGID++)
            {}

            /// Destructor
            virtual ~Identifier() = default;

            /// Get current name.
            const Core::String& getName() const { return _name; }

            /// Set name.
            void setName(const Core::String& name)
            {
                _name = name;
            }

            /// Get current global ID.
            const uint64_t getGID() const { return _gid; }

        protected:
            const uint64_t  _gid;         ///< Global Identifier (UNIQUE)
            Core::String    _name;        ///< Associated name   (NONE UNIQUE BY BUILD)

        private:
            static std::atomic_uint_fast64_t  _latestGID;  ///< [INTERNAL] Provide gID
    };
}