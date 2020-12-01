#pragma once

#include <MouCaCore/include/Core.h>

#include <LibCore/include/CoreThread.h>

namespace Core
{
    class Resource;
    using ResourceSPtr = std::shared_ptr<Resource>;
}

namespace MouCaCore
{
    class LoaderManager;

    //----------------------------------------------------------------------------
    /// \brief Manage a queue to load resource (multi-threading)
    ///
    /// \see   Core::Thread
    class LoadingQueue final : public Core::Thread
    {
        MOUCA_NOCOPY_NOMOVE(LoadingQueue);

        public:
            enum State
            {
                Stop,
                Waiting,
                Running
            };

            /// Constructor
            LoadingQueue():
            _iD(0),
            _run(true),
            _state(Stop),
            _manager(nullptr)
            {}

            /// Destructor
            ~LoadingQueue() = default;

            void initialize(LoaderManager* manager, const size_t iD);

            void release();

            void demandToFinish();

            void addJob(const LoadingItems& items);

            //------------------------------------------------------------------------
            /// \brief  Get current estimation of number of job in queue.
            /// Optimization: We don't protect this vector, so result can't be exact but is nearest of reality.
            /// \returns Estimation of number of job.
            size_t getNumberJobs() const
            {
                return _resources.size();
            }

            State getState() const
            {
                return _state;
            }

        private:
            void run() override;

            void doAction( LoadingItem& item);

            size_t                   _iD;
            bool                     _run;        ///< State of run loop.
            State                    _state;      ///< State of thread.
            LoaderManager*           _manager;    ///< Link to manager.
            
            std::mutex               _jobMutex;
            LoadingItems             _resources;  ///< List of job.
            
            std::mutex               _waitJobMutex;
            std::condition_variable  _waitJob;
    };

    //----------------------------------------------------------------------------
    /// \brief Manage how to load resources (multi-threading)
    ///
    /// \see   LoadingQueue
    class LoaderManager final : public ILoaderManager
    {
        MOUCA_NOCOPY_NOMOVE(LoaderManager);

        public:
            class SynchonizeData
            {
                MOUCA_NOCOPY_NOMOVE( SynchonizeData );

                public:
                    SynchonizeData():
                    _maskThread(0), _countThreadReady(0)
                    {}

                    void initialize(const uint32_t countThreadReady);

                    void synchronize();

                    void threadWorking(const size_t idThread);

                    void threadReady(const size_t idThread);

                private:
                    uint32_t                _maskThread;
                    uint32_t                _countThreadReady;
                    std::mutex              _waitSync;
                    std::condition_variable _wait;
            };

            /// Constructor
            LoaderManager() = default;
            /// Destructor
            ~LoaderManager() override = default;

            void initialize(const uint32_t nbQueue = 5) override;

            void release() override;

            void loadResources(LoadingItems& queue) override;

            void synchronize() override;

            SynchonizeData& getSynchronizeDirect()
            {
                return _syncDirect;
            }

            SynchonizeData& getSynchronizeDeferred()
            {
                return _syncDeferred;
            }

        private:
            std::deque<LoadingQueue> _queues;       ///< List of working threads.

            SynchonizeData           _syncDirect;   ///< Synchronization system for direct job.
            SynchonizeData           _syncDeferred; ///< Synchronization system for indirect job.
    };
}