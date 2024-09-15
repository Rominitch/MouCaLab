#include "Dependencies.h"

#include <LibCore/include/CoreFile.h>
#include <LibCore/include/CoreFileTracker.h>

namespace Core
{

struct Message
{
    public:
        void onEditFile( Resource& )
        {
            _edited = true;
        }

        bool _edited = false;
};

// cppcheck-suppress syntaxError
TEST(CoreFileTracker, api)
{
    auto message = std::make_shared<Message>();

    FileTracker tracker;
    ASSERT_NO_THROW( tracker.signalFileChanged().connectMember<Message>( message.get(), &Message::onEditFile ) );

    Core::FileSPtr file  = std::make_shared<Core::File>( MouCaEnvironment::getOutputPath() / L"myTrackedFile.txt" );
    Core::FileSPtr file2 = std::make_shared<Core::File>( MouCaEnvironment::getOutputPath() / L"myTrackedFile2.txt" );

    auto editFile = [&]( Core::FileSPtr localFile )
    {
        ASSERT_NO_THROW( localFile->open( L"w+" ) );
        ASSERT_NO_THROW( localFile->writeText( "Hello World !" ) );
        ASSERT_NO_THROW( localFile->close() );

        // Dummy sync just to let OS make job
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    };

    EXPECT_FALSE( message->_edited );
    editFile(file);
    editFile(file2);
    EXPECT_FALSE( message->_edited );

    ASSERT_NO_THROW(tracker.registerResource( file ));

    ASSERT_NO_THROW(tracker.startTracking());

    EXPECT_FALSE( message->_edited );
    editFile( file );
    EXPECT_TRUE( message->_edited );
    message->_edited = false; // Reset manually
    
    ASSERT_NO_THROW(tracker.unregisterResource( file ));

    EXPECT_FALSE( message->_edited );
    editFile( file );
    EXPECT_FALSE( message->_edited );

    // Add more file into same folder
    ASSERT_NO_THROW( tracker.registerResource( file2 ) );
    EXPECT_FALSE( message->_edited );
    editFile( file2 );
    EXPECT_TRUE( message->_edited );
    message->_edited = false; // Reset manually

    // Remove all during thread running
    ASSERT_NO_THROW( tracker.unregisterResource( file2 ) );
    ASSERT_NO_THROW( tracker.unregisterResource( file ) );

    EXPECT_FALSE( message->_edited );

    ASSERT_NO_THROW(tracker.stopTracking());
}

}