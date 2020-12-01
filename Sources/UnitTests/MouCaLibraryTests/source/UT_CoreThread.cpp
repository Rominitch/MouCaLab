#include "Dependencies.h"

#include <LibCore/include/CoreThread.h>

using namespace std::chrono_literals;

class DemoThread final : public Core::Thread
{
    private:
        int  _modif;
        bool _end;
        bool _start;

        void run()
        {
            _start = true;
            do
            {
                _info = _modif; // Just copy and sleep !
                std::this_thread::sleep_for(3ms);
            }
            while (!_end && _info != _modif); // Until we stop
        }

    public:
        int _info;

        DemoThread(const int val, const int modif):
        _start(false), _info(val), _modif(modif), _end(false)
        {}
        ~DemoThread() override = default;

        bool isStarted() const { return _start; }

        void demandEnd()
        {
            _end = true;
        }

        int getModif() const
        {
            return _modif;
        }
};

class CrashThread final : public Core::Thread
{
    private:

        void run()
        {
            MOUCA_THROW_ERROR(u8"Crash", u8"Crash");
        }

    public:
        CrashThread() = default;
        ~CrashThread() override = default;
};

TEST(CoreThread, defaultUse)
{
    const int val   = 123;
    const int modif = 321;

    DemoThread myThread(val, modif);
    EXPECT_EQ(val, myThread._info);
    EXPECT_EQ(modif, myThread.getModif());

    ASSERT_NO_THROW(myThread.start());
    
    int retry = 0;
    const int maxRetry = 15; //Security to avoid server locked !
    while(!myThread.isStarted() && retry < maxRetry)
    {
        std::this_thread::sleep_for(2ms);
        ++retry;
    }
    EXPECT_LE(retry, maxRetry);
    EXPECT_FALSE(myThread.isTerminated());
    
    // Finish
    ASSERT_NO_THROW(myThread.demandEnd());

    // Demand to join
    ASSERT_NO_THROW(myThread.join());
    EXPECT_TRUE(myThread.isTerminated());

    // Check value
    EXPECT_EQ(modif, myThread._info);
    EXPECT_EQ(modif, myThread.getModif());
}

//--------------------------------------------------------------------------
// This test will:
//      - Create thread and call run
//      - Thread launch exception and we catch it into main.
//--------------------------------------------------------------------------
TEST(CoreThread, crashInsideThread)
{
    CrashThread thread;

    ASSERT_NO_THROW(thread.start());

    try
    {
        thread.join();
        FAIL() << "Must send exception";
    }
    catch(const Core::Exception& )
    {
    	
    }
    catch (...)
    {
        FAIL() << "Missing exception type";
    }
}