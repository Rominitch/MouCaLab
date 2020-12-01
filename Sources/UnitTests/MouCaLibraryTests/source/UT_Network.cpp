#include "Dependencies.h"

#include <LibNetwork/include/Network.h>

namespace Network
{

struct Message 
{
    Message(uint64_t code = 0ull, int32_t data0 = 0, float data1 = 0.0f, Core::String data2 = Core::String()) :
    _code(code), _data0(data0), _data1(data1), _data2(data2)
    {}

    uint64_t _code;
    int32_t  _data0;
    float      _data1;
    Core::String _data2;
};

class MessageDemo : public IMessagesManager
{
    public:
        MessageDemo() = default;
        ~MessageDemo() override = default;

        void incomingMessage(IMessage& message) override
        {
            _msg._code = message.getCode();
            message.getEditBuffer() >> _msg._data0 >> _msg._data1;
            message.getEditBuffer() >> _msg._data2;
        }

        /// Message receive
        Message _msg;
};

TEST(Network, simpleCommunication)
{
    // Build DEMO manager
    auto manager = std::make_shared<MessageDemo>();

    const Core::String address("127.0.0.1");
    const uint16_t portServer = 13445_i16;
    const uint16_t portClient = 13446_i16;
    Network networkServer;
    EXPECT_BOOST_NO_THROW(networkServer.initialize());
    EXPECT_BOOST_NO_THROW(networkServer.addListener(portServer, manager));

    Network networkClient;
    EXPECT_BOOST_NO_THROW(networkClient.initialize());
    // Connect to server
    EXPECT_BOOST_NO_THROW(networkClient.connectTo(address, portServer));
    // Connect to no-one listening (boost exception)
    EXPECT_ANY_THROW(networkClient.connectTo(address, portClient));

    // Send and check
    {
        const Message data(123ull, 1, 2.0, Core::String("hello"));
        IMessage message(data._code);
        message.getEditBuffer() << data._data0 << data._data1 << data._data2;
        EXPECT_BOOST_NO_THROW(networkClient.sendMessage(address, portServer, message));

        // Locker: just be sure something happens
        uint32_t loop = 0;
        while (loop < 5)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            // Quit when message
            if (manager->_msg._code != 0 )
                break;
            ++loop;
        }

        // Check message receive
        EXPECT_EQ(data._code,  manager->_msg._code);
        EXPECT_EQ(data._data0, manager->_msg._data0);
        EXPECT_EQ(data._data1, manager->_msg._data1);
        EXPECT_EQ(data._data2, manager->_msg._data2);
    }

    EXPECT_BOOST_NO_THROW(networkClient.release());
    EXPECT_BOOST_NO_THROW(networkServer.release());

    manager.reset();
    EXPECT_EQ(0, manager.use_count());
}

}