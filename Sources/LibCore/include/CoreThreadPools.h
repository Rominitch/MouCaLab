/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreThread.h>

namespace Core
{

//----------------------------------------------------------------------------
/// \brief ThreadPool allows to manage ALL threads of application.
/// ThreadPool allows to manage ALL threads of application.
/// It can manage some group too (and provide sync method).
/// \see IThreadPools
class ThreadPools final
{
    MOUCA_NOCOPY_NOMOVE(ThreadPools);
    public:
        ThreadPools() = default;

        ~ThreadPools()
        {
            closeAllThreads();
        }

        size_t createThreadsGroup()
        {
            const size_t currentID = _groupList.size();
            //Build new list for group
            std::vector<Core::ThreadWPtr> newGroup;
            _groupList.emplace_back(newGroup);

            return currentID;
        }

        void addThread(Core::ThreadSPtr pThread)
        {
            _threadList.push_back(pThread);
        }

        void addThread(Core::ThreadSPtr pThread, const size_t groupID)
        {
            _threadList.push_back(pThread);

            //Add thread to group
            BT_ASSERT(groupID < _groupList.size());
            _groupList[groupID].push_back(pThread);
        }

        size_t sizeOfGroup(const size_t groupID) const
        {
            MOUCA_PRE_CONDITION(groupID < _groupList.size());
            return _groupList[groupID].size();
        }

        void waitGroup(const size_t groupID) const
        {
            MOUCA_PRE_CONDITION(groupID < _groupList.size());

            for(const Core::ThreadWPtr& pWThread : _groupList[groupID])
            {
                MOUCA_PRE_CONDITION(!pWThread.expired());	//DEV Issue: impossible to delete thread and call group !!

                //Try to read weak
                Core::ThreadSPtr pThread = pWThread.lock();

                if(!pThread->isTerminated())
                {
                    pThread->join();
                }

                MOUCA_POST_CONDITION(pThread->isTerminated());
            }
        }

        //------------------------------------------------------------------------
        /// \brief  Release thread of pool (MUST be delete definitively after this operation).
        /// 
        /// \param[in,out] pThread: Thread to release (MUST be terminated).
        void release(Core::ThreadWPtr pThread)
        {
            MOUCA_PRE_CONDITION(!pThread.expired()); ///DEV Issue: Impossible to delete empty thread.

            auto thread = pThread.lock();
            MOUCA_PRE_CONDITION(thread.use_count() == 2);
            // Search item
            const auto itDelete = std::find(_threadList.begin(), _threadList.end(), thread);
            // Release it
            _threadList.erase(itDelete);

            thread.reset();
        }

    private:
        void closeAllThreads()
        {
            //Remove all groups (at this step all is lost)
            _groupList.clear();

            //Wait all threads -> can create deadlock if one thread is working infinitly
            auto itThread = _threadList.cbegin();
            while(itThread != _threadList.cend())
            {
                if(!(*itThread)->isTerminated())
                    (*itThread)->join();
                ++itThread;
            }
            _threadList.clear();
        }
    protected:
    #pragma warning(push)
    #pragma warning(disable: 4251)
        std::vector<Core::ThreadSPtr>              _threadList; ///< All thread of application.

        std::vector<std::vector<Core::ThreadWPtr>> _groupList;  ///< All groups of thread.
    #pragma warning(pop)
};

}