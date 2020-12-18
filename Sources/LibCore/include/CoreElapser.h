/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    using ChronoPrecision   = std::chrono::milliseconds;
    using ChronoClock       = std::chrono::high_resolution_clock;
    using ChronoTimePoint   = std::chrono::time_point<ChronoClock>;
    using ChronoDuration    = std::chrono::duration<int64_t>;

    //----------------------------------------------------------------------------
    /// \brief Compute time elapse tick.
    class Elapser
    {
        MOUCA_NOCOPY_NOMOVE(Elapser);

        public:
            /// Constructor
            Elapser() :
            _beforeFrameTimer(ChronoClock::now()),
            _afterFrameTimer(ChronoClock::now())
            {}

            ~Elapser() = default;

            //------------------------------------------------------------------------
            /// \brief  Reset state of elapser
            /// 
            void reset()
            {
                _beforeFrameTimer = ChronoClock::now();
                _afterFrameTimer  = ChronoClock::now();
            }

            //------------------------------------------------------------------------
            /// \brief  Tick allows to catch current time and compute elapsed time between now and latest tick.
            /// 
            template<typename Precision>
            int64_t tick()
            {
                _afterFrameTimer = ChronoClock::now();
                int64_t elapse = std::chrono::duration_cast<Precision>(_afterFrameTimer - _beforeFrameTimer).count();
                _beforeFrameTimer = _afterFrameTimer;

                return elapse;
            }
        private:
            ChronoTimePoint _beforeFrameTimer;
            ChronoTimePoint _afterFrameTimer;
    };
}
