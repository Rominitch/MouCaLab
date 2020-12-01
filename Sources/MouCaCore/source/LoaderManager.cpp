#include "Dependencies.h"

#include "LibCore/include/CoreFile.h"

#include "LibRT/include/RTAnimationBones.h"
#include "LibRT/include/RTImage.h"
#include "LibRT/include/RTShaderFile.h"
#include "LibRT/include/RTMesh.h"

#include "LibMedia/include/AnimationLoader.h"
#include "LibMedia/include/ImageLoader.h"
#include "LibMedia/include/MeshLoader.h"

#include "MouCaCore/include/LoaderManager.h"

namespace MouCaCore
{

// Debug class: enable log + sync finished after timeout
//#define LOADING_DEBUG

void LoaderManager::SynchonizeData::initialize(const uint32_t countThreadReady)
{
    MOUCA_PRE_CONDITION(countThreadReady < 32); //DEV Issue: we don't want more than 32 threads

    std::unique_lock<std::mutex> lock(_waitSync);
    _maskThread = (1 << countThreadReady) - 1;
    // All is ready by default
    _countThreadReady = _maskThread;

#ifdef LOADING_DEBUG
    std::bitset<8> mask(_maskThread);
    std::stringstream ss;
    ss << "Initialize SyncData: " << countThreadReady << " with mask " << mask << std::endl;
    BT_PRINT_MESSAGE(ss.str());
#endif

    MOUCA_POST_CONDITION(_maskThread > 0);
}

void LoaderManager::SynchonizeData::synchronize()
{
    std::unique_lock<std::mutex> lock(_waitSync);
#ifdef LOADING_DEBUG
    // Wait with timeout to debug
    _wait.wait_for(lock, std::chrono::milliseconds(10000),
        [this]
        {

            std::bitset<8> mask(_countThreadReady);
            std::stringstream ss;
            ss << "  Synchronize: " << mask << " " << ((_countThreadReady & _maskThread) == _maskThread ? "SYNC" : "WAIT") << std::endl;
            BT_PRINT_MESSAGE(ss.str());
            return (_countThreadReady & _maskThread) == _maskThread;
        });
#else
    // Wait locker
    _wait.wait(lock, [this] { return (_countThreadReady & _maskThread) == _maskThread; });
#endif
}

void LoaderManager::SynchonizeData::threadWorking(const size_t idThread)
{
    std::unique_lock<std::mutex> lock(_waitSync);
    _countThreadReady &= ~(1 << idThread);
#ifdef LOADING_DEBUG
    std::bitset<8> mask(_countThreadReady);
    std::stringstream ss;
    ss << "  Working: " << idThread << " with mask " << mask << std::endl;
    BT_PRINT_MESSAGE(ss.str());
#endif
}

void LoaderManager::SynchonizeData::threadReady(const size_t idThread)
{
    {
        std::unique_lock<std::mutex> lock(_waitSync);
#ifdef LOADING_DEBUG
        const uint32_t countThreadReady = _countThreadReady | (1 << idThread);
        if (countThreadReady != _countThreadReady)
        {
            std::bitset<8> mask(countThreadReady);
            std::stringstream ss;
            ss << "  Done: " << idThread << " with mask " << mask << std::endl;
            BT_PRINT_MESSAGE(ss.str());
        }
#endif
        _countThreadReady |= (1 << idThread);
    }
    _wait.notify_all();
}

void LoadingQueue::initialize(LoaderManager* manager, const size_t iD)
{
    _manager = manager;
    _iD = iD;
}

void LoadingQueue::release()
{

}

void LoadingQueue::demandToFinish()
{
    // Demand end of loop
    _run = false;
    // Wake up (if case of lock !)
    _waitJob.notify_all();
}

void LoadingQueue::addJob(const LoadingItems& items)
{
    MOUCA_PRE_CONDITION(_run);

    // Register job
    {
        std::unique_lock<std::mutex> locker(_jobMutex);
        for( const auto& item : items )
        {
            _resources.push_back( item );
        }
        // Reorder
        std::sort(_resources.begin(), _resources.end());

        // Take job in direct
        if(_resources.front()._state == LoadingItem::Direct)
            _manager->getSynchronizeDirect().threadWorking(_iD);

        // Take job in deferred
        if(_resources.back()._state == LoadingItem::Deferred)
            _manager->getSynchronizeDeferred().threadWorking(_iD);
    }

    // Signal job
    _waitJob.notify_all();
}

void LoadingQueue::doAction( LoadingItem& item )
{
    // Todo: read resource + prepare ?
    RT::ShaderFile* file = dynamic_cast<RT::ShaderFile*>(item._resource.get());
    if( file != nullptr )
    {
        MOUCA_ASSERT( !file->getFilename().empty() );
        MOUCA_ASSERT( std::filesystem::exists( file->getFilename() ) );

        file->open(L"rb");
        return;
    }

    RT::ImageImport* image = dynamic_cast<RT::ImageImport*>(item._resource.get());
    if( image != nullptr )
    {
        MOUCA_ASSERT( !image->getFilename().empty() );
        MOUCA_ASSERT( std::filesystem::exists( image->getFilename() ) );
        Media::ImageLoader loader;
        image->setImage(loader.openImage(image->getFilename()));
        return;
    }
    
    RT::MeshImport* importer = dynamic_cast<RT::MeshImport*>(item._resource.get());
    if( importer != nullptr )
    {
        Media::MeshLoader loader;
        loader.createMesh( *importer );
        return;
    }

    RT::AnimationImporter* animation = dynamic_cast<RT::AnimationImporter*>(item._resource.get());
    if( animation != nullptr )
    {
        Media::AnimationLoader loader;
        loader.createAnimation(*animation);
        return;
    }
    
    MOUCA_THROW_ERROR( "MouCaCore", "InvalidLoader" );
}

void LoadingQueue::run()
{
    while(_run)
    {
        // Check resource state
        if( _resources.empty() )
        {
            _state = Waiting;

            // Signal no job !
            {
                std::unique_lock<std::mutex> locker( _waitJobMutex );
                if( _resources.empty() )
                {
                    _manager->getSynchronizeDirect().threadReady( _iD );
                    _manager->getSynchronizeDeferred().threadReady( _iD );
                }
            }

            // Wait quasi-passive
            std::unique_lock<std::mutex> locker( _waitJobMutex );
            _waitJob.wait_for( locker, std::chrono::milliseconds( 240 ), [this]{ return !_resources.empty(); } );

            _state = Running;
        }
        else
        {
            LoadingItem item;

            // Take job
            {
                std::unique_lock<std::mutex> locker( _jobMutex );
                item = std::move(_resources.front());
                _resources.pop_front();
            }
            
            try
            {
                doAction( item );
            }
            catch(...)
            {
            	// When resource can't be read/extract !
            }

            // Signal no direct job left
            {
                std::unique_lock<std::mutex> locker( _jobMutex );
                if( !_resources.empty() && _resources.front()._state != LoadingItem::Direct )
                {
                    _manager->getSynchronizeDirect().threadReady( _iD );
                }
            }
        }
    }
    _state = Stop;

    // Missing synchronize or strong shutdown !
    MOUCA_POST_CONDITION(_resources.empty());
}

void LoaderManager::initialize(const uint32_t nbQueues)
{
    MOUCA_PRE_CONDITION(_queues.empty());  // DEV Issue: call initialize() both time.
    MOUCA_PRE_CONDITION(nbQueues > 0);     // DEV Issue: Need minimum of queue !

    // Allocate synchronizer
    _syncDirect.initialize( nbQueues );
    _syncDeferred.initialize( nbQueues );

    // Create queue
    _queues.resize(nbQueues);

    // Launch thread
    size_t id=0;
    for(auto& queue : _queues)
    {
        queue.initialize(this, id);
        queue.start();
        ++id;
    }

    // Wait all thread ready to make job
    // Avoid dead lock when thread don't receive wait signal.
    synchronize();

    MOUCA_POST_CONDITION(!_queues.empty()); /// Operation Failed ?
}

void LoaderManager::release()
{
    MOUCA_PRE_CONDITION(!_queues.empty()); // DEV Issue: call release before initialize().

    // Send order
    for(auto& queue : _queues)
    {
        queue.demandToFinish();
    }

    // Wait all
    for(auto& queue : _queues)
    {
        queue.join();
        queue.release();
    }

    // Remove all
    _queues.clear();

    MOUCA_POST_CONDITION(_queues.empty()); /// Operation Failed ?
}

void LoaderManager::loadResources(LoadingItems& items)
{
    MOUCA_PRE_CONDITION(!_queues.empty()); // DEV Issue: Missing call initialize() !
    MOUCA_PRE_CONDITION(!items.empty());   // DEV Issue: No job ?

    // Sort queue to have priority job first !
    std::sort(items.begin(), items.end());

    // Algo 1: Job for all
    //  We add to each queue an item until we finish
    auto itQueue = _queues.begin();
    while(!items.empty())
    {
        LoadingItems local;
        // Transfer to local
        local.push_back(std::move(items.front()));
        items.pop_front();
        
        itQueue->addJob( local );
        ++itQueue;

        // Restart thread
        if(itQueue == _queues.end())
            itQueue = _queues.begin();
    }

    // Wait "direct" resource before leave
    _syncDirect.synchronize();
}

void LoaderManager::synchronize()
{
    // Wait all
    _syncDeferred.synchronize();
}

}