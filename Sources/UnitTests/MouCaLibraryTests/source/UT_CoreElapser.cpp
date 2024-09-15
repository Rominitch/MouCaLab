#include "Dependencies.h"

#include <LibCore/include/CoreElapser.h>


TEST(CoreElapser, compute)
{
    Core::ElapserCP timer;
    std::this_thread::sleep_for(std::chrono::milliseconds( 30 )); // Not a really precise time spender
    int64_t e0 = timer.tick();
    EXPECT_BETWEEN(30, 60, e0); // Set high limit to check when issue But min mustn't be reached.

    std::this_thread::sleep_for(std::chrono::milliseconds( 40 ));
    int64_t e = timer.tick();
    EXPECT_BETWEEN(40, 70, e);

    timer.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds( 30 ));
    e = timer.tick();
    EXPECT_BETWEEN(30000, 60000, e);
}

TEST(CoreElapser, elapse)
{
    Core::ElapserCP timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Not a really precise time spender
    int64_t e0 = timer.elapse();
    EXPECT_BETWEEN(30, 60, e0); // Set high limit to check when issue But min mustn't be reached.

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    int64_t e = timer.elapse();
    EXPECT_BETWEEN(70, 90, e);
}