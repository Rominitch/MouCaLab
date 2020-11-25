#include "Dependancies.h"

#include "LibCore/include/CoreSignal.h"

namespace Core
{

struct Receiver
{
    void doSomething(int number)
    {
        _doIt = number;
    }

    int _doIt = 0;
};

// cppcheck-suppress syntaxError
TEST(CoreSignal, checkAPI)
{
    Core::Signal<int> mySignal;
    Receiver receiver;
    Receiver receiverAnother;
    std::shared_ptr<Receiver> receiverSPtr = std::make_shared<Receiver>();
    EXPECT_EQ(0, receiver._doIt);
    EXPECT_EQ(0, receiverAnother._doIt);
    EXPECT_EQ(0, receiverSPtr->_doIt);

    // Connect to signal
    EXPECT_FALSE(mySignal.isConnected(&receiver));
    ASSERT_NO_THROW(mySignal.connectMember<Receiver>( &receiver, &Receiver::doSomething ));
    EXPECT_TRUE(mySignal.isConnected(&receiver));
    ASSERT_NO_THROW(mySignal.connectMember<Receiver>( &receiverAnother, &Receiver::doSomething ) );
    
    auto weakReceiver = std::weak_ptr<Receiver>(receiverSPtr);
    EXPECT_FALSE(mySignal.isConnected(weakReceiver));
    ASSERT_NO_THROW(mySignal.connectWeakMember<Receiver>(weakReceiver, &Receiver::doSomething));
    EXPECT_TRUE(mySignal.isConnected(weakReceiver));

    // Emit and check new value
    const int value = 42;
    ASSERT_NO_THROW(mySignal.emit( value ));
    EXPECT_EQ(value, receiver._doIt);
    EXPECT_EQ(value, receiverAnother._doIt);
    EXPECT_EQ(value, receiverSPtr->_doIt);

    // Disconnect specific and check nothing receive for first
    const int valueAnother = 311;
    ASSERT_NO_THROW( mySignal.disconnect(&receiver) );
    ASSERT_NO_THROW( mySignal.emit( valueAnother ) );
    EXPECT_EQ(value,        receiver._doIt);
    EXPECT_EQ(valueAnother, receiverAnother._doIt);
    EXPECT_EQ(valueAnother, receiverSPtr->_doIt);

    // Connect again
    ASSERT_NO_THROW( mySignal.connectMember<Receiver>( &receiver, &Receiver::doSomething ) );

    // Emit and check new value
    const int valueFinal = 4;
    ASSERT_NO_THROW( mySignal.emit( valueFinal ) );
    EXPECT_EQ(valueFinal, receiver._doIt);
    EXPECT_EQ(valueFinal, receiverAnother._doIt);
    EXPECT_EQ(valueFinal, receiverSPtr->_doIt);

    // Disconnect all and check do nothing for all
    ASSERT_NO_THROW( mySignal.disconnectAll() );
    ASSERT_NO_THROW( mySignal.emit( value ) );
    EXPECT_EQ(valueFinal, receiver._doIt);
    EXPECT_EQ(valueFinal, receiverAnother._doIt);
    EXPECT_EQ(valueFinal, receiverSPtr->_doIt);
}

}