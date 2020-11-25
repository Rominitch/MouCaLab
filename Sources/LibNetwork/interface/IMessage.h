/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreByteBuffer.h>

namespace Network
{
    class IMessage
    {
        MOUCA_NOCOPY_NOMOVE(IMessage);

        public:
            struct Code
            {
                static const uint64_t CodeMessageMask            = 0x0000FFFF;

                // Server connection
                static const uint64_t ServerMask                 = 0x00010000;
                static const uint64_t DemandConnectionServer     = ServerMask | 0x00000001;
                static const uint64_t ValidationConnectionServer = ServerMask | 0x00000002;

            };

            IMessage(const void* buffer, const uint64_t sizeBuffer):
            _code(0)
            {
                MOUCA_PRE_CONDITION(sizeBuffer >= 8); // DEV Issue: Message too small: need code.
                const char* data = reinterpret_cast<const char*>(buffer);

                // Read code
                memcpy(&_code, data, sizeof(uint64_t));

                // Read end of message
                if(sizeBuffer > 8)
                {
                    _buffer.readBuffer(&data[8], sizeBuffer - 8);
                }
            }

            IMessage(const uint64_t code):
            _code(code)
            {
                _buffer << _code;
            }

            virtual ~IMessage() = default;
        
            uint64_t            getCode() const   { return _code; }

            Core::ByteBuffer&       getEditBuffer()   { return _buffer; }

            const Core::ByteBuffer& getBuffer() const { return _buffer; }

        protected:
            uint64_t     _code;
            Core::ByteBuffer _buffer;
    };

    //----------------------------------------------------------------------------
    /// \brief Abstract class to manage all incoming message.
    /// 
    class IMessagesManager
    {
        MOUCA_NOCOPY_NOMOVE(IMessagesManager);

        public:
            /// Destructor
            virtual ~IMessagesManager() = default;
            
            //------------------------------------------------------------------------
            /// \brief  Manage the incoming message to make something.
            /// 
            /// \param[inOut] message: data to manage.
            virtual void incomingMessage(IMessage& message) = 0;

        protected:
            /// Constructor
            IMessagesManager() = default;
    };

    using IMessagesManagerSPtr = std::shared_ptr<IMessagesManager>;
    using IMessagesManagerWPtr = std::weak_ptr<IMessagesManager>;

}