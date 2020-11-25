#include "Dependancies.h"

#include <LibCore/include/CoreThread.h>
#include <LibCore/include/CoreThreadPools.h>

using namespace std::chrono_literals;

class DemoThreadPools : public Core::Thread
{
private:
    const int _modif;
    bool _end;
    std::atomic<bool> _isStarted;

    void run() override
    {
        _isStarted.store(true);        
        do
        {
            _info = _modif; // Just copy and sleep !
            std::this_thread::sleep_for(3ms);
        }
        while (!_end); // Until we stop
        _isStarted.store(false);
    }

public:
    int _info;

    DemoThreadPools(const int val, const int modif) :
    _isStarted(false), _info(val), _modif(modif), _end(false)
    {}

    ~DemoThreadPools()
    {}

    bool isStarted() const { return _isStarted.load(); }

    void demandEnd()
    {
        _end = true;
    }

    int getModif() const
    {
        return _modif;
    }
};

TEST(CoreThreadPools, addThread)
{
    auto thread0 = std::make_shared<DemoThreadPools>(1, 2);
    {
        Core::ThreadPools pools;

        auto thread1 = std::make_shared<DemoThreadPools>(3, 4);
        auto thread2 = std::make_shared<DemoThreadPools>(5, 6);

        thread0->start();
        thread1->start();
        thread2->start();

        // Attach thread to pool
        ASSERT_NO_THROW(pools.addThread(thread0));

        // Build new group
        size_t group = 0;
        ASSERT_NO_THROW(group = pools.createThreadsGroup());
        ASSERT_NO_THROW(pools.addThread(thread1, group));
        ASSERT_NO_THROW(pools.addThread(thread2, group));
        EXPECT_EQ(2, pools.sizeOfGroup(group));

        // Wait all thread are inside run()
        int retry = 0;
        const int maxRetry = 15;
        while( retry < maxRetry )
        {
            if (thread0->isStarted() && thread1->isStarted() && thread2->isStarted())
                break;
            std::this_thread::sleep_for(3ms);
            ++retry;
        }
        EXPECT_LE(retry, maxRetry);
        ASSERT_TRUE(thread0->isStarted());
        ASSERT_TRUE(thread1->isStarted());
        ASSERT_TRUE(thread2->isStarted());

        // Demand to finish
        ASSERT_NO_THROW(thread0->demandEnd());
        ASSERT_NO_THROW(thread1->demandEnd());
        ASSERT_NO_THROW(thread2->demandEnd());

        // Sync
        ASSERT_NO_THROW(pools.waitGroup(group));

        // Execution is done properly
        ASSERT_FALSE(thread1->isStarted()) << "waitGroup doesn't make job ?";
        ASSERT_FALSE(thread2->isStarted()) << "waitGroup doesn't make job ?";
        EXPECT_EQ(thread1->getModif(), thread1->_info);
        EXPECT_EQ(thread2->getModif(), thread2->_info);
    }

    EXPECT_EQ(thread0->getModif(), thread0->_info);
}