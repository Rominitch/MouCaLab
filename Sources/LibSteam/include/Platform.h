/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Steam
{
    class Platform final
    {
        public:
            Platform();
            ~Platform();

            void initialize();
            void release();

            bool isNull() const;

        private:
            bool       _initialize;
    };

}
