/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Tools based on c++11 for manage thread manipulation.
    /// Tools based on c++11 for manage thread manipulation.
    /// The main objective is to embedded data easily.
    /// Currently, Thread report exception to caller when call join().
    ///
    /// \code{.cpp}
    ///     MyThread thread;
    ///     // --> My operation for initialize thread.
    ///     thread.start();
    ///     // --> My current thread live.
    ///     thread.join();
    ///     // --> Here thread can be delete/forget
    /// \endcode
    /// \see   std::thread
    class Thread
    {
        MOUCA_NOCOPY(Thread);

        protected:
            //------------------------------------------------------------------------
            /// \brief  Methods execute by thread. Must be inherit.
            /// 
            virtual void run()=0;

            /// Constructor (ONLY for inherit class).
            Thread():
            _latestException(nullptr)
            {
                MouCa::preCondition(isNull());
            }
            
        public:
            /// Destructor
            virtual ~Thread()
            {
                // Have you call join() thread before remove object ?
                MouCa::preCondition(isNull() || isTerminated());
            }

            //------------------------------------------------------------------------
            /// \brief  Check if thread is null.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const
            {
                return _pThread == nullptr;
            }

            //------------------------------------------------------------------------
            /// \brief  Launch thread.
            ///
            /// \note Can be launched only one time.
            void start()
            {
                MouCa::preCondition(isNull());  //DEV Issue: Second start() calling !
                _pThread = std::unique_ptr<std::thread>(new std::thread(runThread, this));
                MouCa::postCondition(!isNull());
            }

            //------------------------------------------------------------------------
            /// \brief  Check if current thread is terminated.
            /// 
            /// \returns True if thread is completely finished, false otherwise.
            /// \note This method is possible ONLY after start().
            bool isTerminated() const
            {
                MouCa::preCondition(!isNull());    //DEV Issue: Need start() before
                return !(_pThread->joinable());
            }

            //------------------------------------------------------------------------
            /// \brief  Wait until thread finish this task.
            ///
            /// \note This method hasn't timeout so VERIFY DEAD LOCK.
            /// \note This method is possible ONLY after start().
            void join()
            {
                MouCa::preCondition(!isNull());    //DEV Issue: Need start() before
                _pThread->join();

                // Rethrow exception inside thread
                if(_latestException != nullptr)
                {
                    std::rethrow_exception(_latestException);
                }
            }

        private:
            std::unique_ptr<std::thread> _pThread;           ///< Thread instance
            std::exception_ptr           _latestException;   ///< Propagate c++ exception.

            //------------------------------------------------------------------------
            /// \brief  Static method to call run() of thread instance.
            /// 
            /// \param[in] pThread: instance where to call run().
            /// \note This method manages exception transfer. Using join() rethrow will be called, if exception happens.
            static void runThread(Thread* pThread)
            {
                MouCa::preCondition(pThread != NULL);
                try
                {
                    //Run Thread methods
                    pThread->run();
                }
                catch(...)
                {
                    // Keep latest exception for caller (C++ 11 propagation)
                    pThread->_latestException = std::current_exception();
                }
            }
    };

    using ThreadSPtr = std::shared_ptr<Thread>;
    using ThreadWPtr = std::weak_ptr<Thread>;
}
