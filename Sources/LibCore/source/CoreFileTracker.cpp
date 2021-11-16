/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibCore/include/CoreError.h"
#include "LibCore/include/CoreException.h"
#include "LibCore/include/CoreResource.h"
#include "LibCore/include/CoreFileTracker.h"

namespace Core
{

FileTracker::FileTracker():
_thread(std::make_shared<FileTracker::TrackerThread>(*this))
{}

FileTracker::~FileTracker()
{
    MouCa::preCondition( _thread->isNull() || _thread->isTerminated() );
}

void FileTracker::registerResource( Core::ResourceSPtr resource )
{
    MouCa::preCondition( _thread != nullptr );
    _thread->registerResource(resource);
}

void FileTracker::unregisterResource( Core::ResourceSPtr resource )
{
    MouCa::preCondition( _thread != nullptr );
    _thread->unregisterResource(resource);
}

void FileTracker::startTracking()
{
    MouCa::preCondition( _thread != nullptr );
    MouCa::preCondition( _thread->isNull() );
    _thread->ready();
    _thread->start();

    // Synchronize (WARNING: infinite loop sync)
    while(!_thread->isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    MouCa::postCondition( !_thread->isTerminated() );
}

void FileTracker::stopTracking()
{
    MouCa::preCondition( !_thread->isTerminated() );
    _thread->finish();
    _thread->join();

    MouCa::postCondition( _thread->isTerminated() );
}

FileTracker::TrackerThread::TrackerThread( FileTracker& manager ) :
_manager(manager), _threadAlive( false ), _updateHandleList(false), _isRunning(false)
{
    _waitHandle.reserve( MAXIMUM_WAIT_OBJECTS );
}

void FileTracker::TrackerThread::registerResource( Core::ResourceSPtr resource )
{
    MouCa::preCondition( resource != nullptr && !resource->getTrackedFilename().empty() ); // DEV Issue: Need valid resource !
    MouCa::preCondition( _tracked.size() < MAXIMUM_WAIT_OBJECTS );                         // DEV Issue: 64 item tracked for one wait !

    // Protect Multi-threading access
    std::lock_guard<std::mutex> guard( _lockTracked );

    // Extract folder    
    const Core::Path& path = resource->getTrackedFilename();
    const Core::StringOS folder = path.parent_path().wstring();

    // Search if path tracking exists
    auto itExist = std::find_if( _tracked.begin(), _tracked.end(), [&]( const std::pair<key, std::vector<FileInfo>>& data ) { return data.first._trackedPath == folder; } );
    if( itExist != _tracked.end() )
    {
        // Already added ?
        if( std::find_if(itExist->second.cbegin(), itExist->second.cend(), [&](const FileInfo& file) { return file._data.lock().get() == resource.get(); }) == itExist->second.cend() )
        {
            itExist->second.emplace_back(std::move(FileInfo(resource)));
        }
    }
    else
    {
        // Add new resources + tracking system
        MouCa::assertion( folder.size() < MAX_PATH );
        const HANDLE modifyHandle = FindFirstChangeNotification( folder.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE );
        if( modifyHandle == INVALID_HANDLE_VALUE )
        {
            throw Core::Exception(Core::ErrorData("Tools", "FileTrackingError") << resource->getTrackedFilename().string() );
        }

        const key newKey = {modifyHandle, folder};
        std::vector<FileInfo> files;
        files.emplace_back( std::move( FileInfo( resource )) );
        _tracked[newKey] = files;
        _updateHandleList = true;
    }

    MouCa::postCondition( std::find_if( _tracked.begin(), _tracked.end(), [&]( const std::pair<key, std::vector<FileInfo>>& data ) { return data.first._trackedPath == folder; } ) != _tracked.end() ); // DEV Issue: folder is tracked now !
}

void FileTracker::TrackerThread::unregisterResource( Core::ResourceSPtr resource )
{
    MouCa::preCondition( resource != nullptr && !resource->getTrackedFilename().empty() );    // DEV Issue: Need valid resource !

    // Protect Multi-threading access
    std::lock_guard<std::mutex> guard( _lockTracked );

    // Remove resource
    auto itTracked = _tracked.begin();
    while( itTracked != _tracked.end() )
    {
        auto itFile = std::find_if( itTracked->second.begin(), itTracked->second.end(), [&]( const FileInfo& info ) { return info._data.lock() == resource; } );
        if( itFile != itTracked->second.end() )
        {
            // Erase all
            if( itTracked->second.size() == 1 )
            {
//                 auto itHandle = std::find_if( _waitHandle.begin(), _waitHandle.end(), [&]( const HANDLE& handle ) { return handle == itTracked->first._handle; } );
//                 MouCa::assertion( itHandle != _waitHandle.end() );
//                 _waitHandle.erase( itHandle );
                _updateHandleList = true;
                itTracked = _tracked.erase( itTracked );
            }
            else // Just erase item into list
            {
                itTracked->second.erase( itFile );
            }
            break;
        }
        ++itTracked;
    }
}

void FileTracker::TrackerThread::run()
{
    while( _threadAlive )
    {
        _isRunning = true; // Set thread is currently running (allow to sync properly)

        DWORD dwWaitStatus; 
        
        // Check state all 3 seconds
        {
            if( _updateHandleList )
            {
                std::lock_guard<std::mutex> guard( _lockTracked );
                _waitHandle.clear();
                for( auto& tracked : _tracked )
                {
                    _waitHandle.emplace_back( tracked.first._handle );
                }
                _updateHandleList = false;
            }
            dwWaitStatus = WaitForMultipleObjects( static_cast<DWORD>(_waitHandle.size()), _waitHandle.data(), FALSE, _timeTrackMs );
        }

        // Search main issue
        if( dwWaitStatus == WAIT_TIMEOUT || dwWaitStatus == WAIT_FAILED )
        {}
        else
        {
            // Protect Multi-threading access
            std::lock_guard<std::mutex> guard( _lockTracked );

            // Analyze tracked resources
            DWORD count = 0;
            for( const auto& handle : _waitHandle )
            {
                if( dwWaitStatus == WAIT_OBJECT_0 + count )
                {
                    if( FindNextChangeNotification( handle ) == FALSE )
                    {
                        throw Core::Exception(Core::ErrorData( "Tools", "FileTrackingError" ));
                    }

                    if( !_tracked.empty() )
                    {
                        auto itTracked = std::find_if( _tracked.begin(), _tracked.end(), [&]( const std::pair<key, std::vector<FileInfo>>& data ) { return data.first._handle == handle; } );
                        for( auto& fileInfo : itTracked->second )
                        {
                            MouCa::assertion( !fileInfo._data.expired() );
                            ResourceSPtr resource = fileInfo._data.lock();
                            // Check if this file has same edited time than before
                            const auto newTime = std::filesystem::last_write_time( resource->getTrackedFilename() );
                            if( newTime != fileInfo._lastEdited )
                            {
                                MouCa::logConsole(std::format("FileTracker: {} has changed.", resource->getTrackedFilename().string()));

                                // Update time
                                fileInfo._lastEdited = newTime;

                                // Send to other resource is modified
                                _onFileChanged.emit(*resource.get());
                            }
                        }
                    }
                }
                ++count;
            }
        }
    }
    _isRunning = false; // Out of loop
}

}