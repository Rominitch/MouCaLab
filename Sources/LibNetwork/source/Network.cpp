/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibNetwork/include/Network.h"

namespace Network
{

//#define BUFFER_MODE

//#define DUMP_MESSAGE

#ifdef DUMP_MESSAGE
#   define LOG_MESSAGE(msg) std::cout << msg << std::endl
#else
#   define LOG_MESSAGE(msg) void(0)
#endif

ConnectionTCP::ConnectionTCP(boost::asio::io_service& ioService):
_linkIOService(ioService), _socketTCP(ioService), _strand(ioService)
{
    _buffer.resize(1024);
}

ConnectionTCP::~ConnectionTCP()
{
    _socketTCP.close();
    _messagesManager.reset();
}

void ConnectionTCP::bind(const Core::String& ip, const uint16_t port)
{
    MouCa::preCondition(shared_from_this().get() != nullptr);             // Dev Issue: Not created by make_shared !

    // Build end point
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port);

    _socketTCP.connect(endpoint);
}

void ConnectionTCP::send(IMessage& messageByte)
{
    MouCa::preCondition(shared_from_this().get() != nullptr);             // Dev Issue: Not created by make_shared !

    MouCa::preCondition(!_linkIOService.stopped());

    boost::asio::async_write(_socketTCP, boost::asio::buffer(messageByte.getBuffer().getData(), messageByte.getBuffer().size()),
                             [&](const boost::system::error_code& error, const size_t transfered)
                             {
                                 handleWrite(error, transfered);
                             });
}

void ConnectionTCP::receive()
{
    MouCa::preCondition(shared_from_this().get() != nullptr);             // Dev Issue: Not created by make_shared !

    LOG_MESSAGE("Receive: Start");

    if(!_linkIOService.stopped())
    {
#ifdef BUFFER_MODE
        boost::asio::async_read_until(_socketTCP,
                                      _inPacket,
                                      '\0',
                                      [me = shared_from_this()](const boost::system::error_code& error, size_t transfered)
                                      {
                                          me->handleRead(error, transfered);
                                      });
#else
        _socketTCP.async_read_some(
                                boost::asio::buffer(_buffer.data(), _buffer.size()),
                                [me = shared_from_this()](const boost::system::error_code& error, size_t transfered)
                                {
                                    me->handleRead(error, transfered);
                                });
#endif
    }
    LOG_MESSAGE("Receive: End");
}

void ConnectionTCP::accept()
{
    receive();
}

void ConnectionTCP::handleWrite(const boost::system::error_code& error, const size_t /*transfered*/)
{
    if (error)
    {
        LOG_MESSAGE(error.message());
        throw Core::Exception(Core::ErrorData("Network", "WriteError"));
    }
}

void ConnectionTCP::handleRead(const boost::system::error_code& error, const size_t transfered)
{
    LOG_MESSAGE("HandleRead: Start");

    if(!_linkIOService.stopped() && error != boost::asio::error::eof)
    {
        if (error)
        {
            LOG_MESSAGE(error.message());
            throw Core::Exception(Core::ErrorData("Network", "ReadError"));
        }

        // Block: To delete IMessage and release lock before launch another receive
        {
            auto messagesManager = _messagesManager.lock();
            MouCa::preCondition(messagesManager != nullptr); // DEV Issue: need manager to make something of message.

#ifdef BUFFER_MODE
            IMessage message(_inPacket.data().data(), _inPacket.data().size());
#else
            IMessage message(_buffer.data(), transfered);
#endif
            messagesManager->incomingMessage(message);
        }

        receive();
    }

    LOG_MESSAGE("HandleRead: End");
}

Network::Network():
_runner(*this), _work(_IOService)
{}

void Network::initialize()
{
    _runner.start();
}

void Network::release()
{
    // Sync thread
    _IOService.stop();
    _runner.join();

    // Remove memory
    _connections.clear();
    _acceptors.clear();
}

void Network::addListener(const uint16_t port, IMessagesManagerWPtr manager)
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    auto acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(_IOService);
    acceptor->open(endpoint.protocol());
    acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
    acceptor->bind(endpoint);
    acceptor->listen(boost::asio::socket_base::max_connections);
    
    // Start handle
    ConnectionTCPSPtr emptyConnect = nullptr;
    handleAccept(acceptor, manager, emptyConnect, boost::system::error_code());

    _acceptors.emplace_back(acceptor);
}

void Network::handleAccept(AcceptorWPtr acceptorWeak, IMessagesManagerWPtr manager, ConnectionTCPSPtr connect, const boost::system::error_code& error)
{
    LOG_MESSAGE("handleAccept: Start");

    auto acceptor = acceptorWeak.lock();
    MouCa::preCondition(acceptor != nullptr);
    MouCa::preCondition(manager.lock() != nullptr);

    if (!_IOService.stopped())
    {
        if (error)
            throw Core::Exception(Core::ErrorData("Network", "AcceptError"));

        // When valid connection
        if (connect != nullptr && connect->getSocket().is_open())
        {
            connect->registerMessageManager(manager.lock());
            connect->accept();
        }

        // Restart service
        auto newConnect = std::make_shared<ConnectionTCP>(_IOService);
        acceptor->async_accept(newConnect->getSocket(),
            [=](const boost::system::error_code& error)
            {
                handleAccept(acceptorWeak, manager, newConnect, error);
            });
    }

    LOG_MESSAGE("handleAccept: End");
}

void Network::connectTo(const Core::String& ip, const uint16_t port)
{
    ConnectionTCPSPtr tcpConnect = std::make_shared<ConnectionTCP>(_IOService);
    tcpConnect->bind(ip, port);

    _connections.emplace_back(tcpConnect);
}

void Network::sendMessage(const Core::String& ip, const uint16_t port, IMessage& message)
{
    // Search connection
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
    auto itConnect = std::find_if(_connections.cbegin(), _connections.cend(),
        [&](const ConnectionTCPSPtr& connect)
        {
            return connect->isValid(endpoint);
        });

    // Send message
    if(itConnect != _connections.cend())
    {
        (*itConnect)->send(message);
    }
}

void Network::ServiceRunner::run()
{
    LOG_MESSAGE("Start thread");

    _linkNetwork.getIOService().run();

    LOG_MESSAGE("End thread");
}

}