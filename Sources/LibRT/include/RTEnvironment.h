/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Abstract environment.
    /// 
    class Environment
    {
        MOUCA_NOCOPY_NOMOVE(Environment);
        public:
            virtual ~Environment() = default;
 
            virtual bool isNull() const = 0;
            virtual const void* getGenericInstance() const = 0;

        protected:
            Environment() = default;
    };

}