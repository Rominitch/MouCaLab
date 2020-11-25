/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreThread.h>

#include <LibNetwork/interface/IMessage.h>

namespace Network
{
    using AcceptorSPtr = std::shared_ptr<boost::asio::ip::tcp::acceptor>;
    using AcceptorWPtr = std::weak_ptr<boost::asio::ip::tcp::acceptor>;
    using SocketSPtr   = std::shared_ptr<boost::asio::ip::tcp::socket>;

    class ConnectionTCP : public std::enable_shared_from_this<ConnectionTCP>
    {
        public:
            ConnectionTCP(boost::asio::io_service& ioService);
            ~ConnectionTCP();

            void bind(const Core::String& ip, const uint16_t port);

            void send(IMessage& message);

            void receive();

            void accept();
            
            bool isValid(const boost::asio::ip::tcp::endpoint& endpoint) const
            {
                return _socketTCP.remote_endpoint() == endpoint;
            }

            boost::asio::ip::tcp::socket& getSocket() { return _socketTCP; }

            void registerMessageManager(IMessagesManagerWPtr messagesManager)
            {
                _messagesManager = messagesManager;
            }

        private:
            void handleWrite(const boost::system::error_code& error, const size_t transfered);

            void handleRead(const boost::system::error_code& error, const size_t transfered);


            boost::asio::io_service&        _linkIOService;
            boost::asio::ip::tcp::socket    _socketTCP;
            std::vector<char>               _buffer;
            boost::asio::io_service::strand _strand;
            boost::asio::streambuf          _inPacket;

            IMessagesManagerWPtr            _messagesManager;

//             boost::asio::deadline_timer     m_timer;
//             boost::posix_time::ptime        m_last_time;
// 
//             std::vector< uint8_t > m_recv_buffer;
//             std::list< int32_t > m_pending_recvs;
//             std::list< std::vector< uint8_t > > m_pending_sends;
//             int32_t m_receive_buffer_size;
//             int32_t m_timer_interval;
//             volatile uint32_t m_error_state;
    };
    using ConnectionTCPSPtr = std::shared_ptr<ConnectionTCP>;

    class Network
    {
        MOUCA_NOCOPY_NOMOVE(Network);
        
        public:
            Network();

            void initialize();

            void release();

            void addListener(const uint16_t port, IMessagesManagerWPtr manager);

            void connectTo(const Core::String& ip, const uint16_t port);

            void sendMessage(const Core::String& ip, const uint16_t port, IMessage& message);

            boost::asio::io_service& getIOService() { return _IOService; }

        private:
            class ServiceRunner : public Core::Thread
            {
                MOUCA_NOCOPY_NOMOVE(ServiceRunner);
                public:
                    ServiceRunner(Network& linkNetwork):
                    _linkNetwork(linkNetwork)
                    {}

                    ~ServiceRunner() override = default;

                    void run() override;

                private:
                    Network& _linkNetwork;
            };

            void handleAccept(AcceptorWPtr acceptor, IMessagesManagerWPtr manager, ConnectionTCPSPtr connect, const boost::system::error_code& error);

            boost::asio::io_service       _IOService;
            boost::asio::io_service::work _work;
            ServiceRunner                 _runner;

            std::vector<ConnectionTCPSPtr> _connections;
            std::vector<AcceptorSPtr>      _acceptors;

    };

}