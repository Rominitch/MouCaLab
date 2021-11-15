/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreResource.h>
#include <LibCore/include/CoreSignal.h>
#include <LibCore/include/CoreString.h>
#include <LibCore/include/CoreThread.h>

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Allow to track event about some resource file.
    ///
    class FileTracker final
    {
        MOUCA_NOCOPY_NOMOVE( FileTracker );

        public:
            class TrackerThread : public Thread
            {
                public:
                    TrackerThread( FileTracker& manager );

                    void ready()   { _threadAlive = true; }
                    void finish()  { _threadAlive = false; }

                    bool isRunning() const { return _isRunning; }

                    void run() override;

                    void registerResource( ResourceSPtr resource );
                    void unregisterResource( ResourceSPtr resource );

                    Signal<Resource&>& signalFileChanged()
                    {
                        return _onFileChanged;
                    }

                private:
                    static const DWORD _timeTrackMs = 3000; ///< 3 seconds.
                    FileTracker&       _manager;            ///< Link to manager.
                    bool               _threadAlive;        ///< Indicate to thread if user want operation.
                    bool               _isRunning;          ///< Check if threading is running.
                    Signal<Resource&>  _onFileChanged;      ///< Signal system: file changing signal to connected item.

                    struct key
                    {
                        HANDLE   _handle;
                        StringOS _trackedPath;

                        bool operator<(const key& compare) const
                        {
                            return _handle < compare._handle;
                        }
                    };

                    struct FileInfo
                    {
                        ResourceWPtr                    _data;
                        std::filesystem::file_time_type _lastEdited;

                        FileInfo( ResourceSPtr resource ):
                        _data(resource),
                        _lastEdited(std::filesystem::last_write_time( std::filesystem::path( _data.lock()->getTrackedFilename() ) ))
                        {}

                        /*
                        FileInfo( FileInfo&& info ) :
                        _data(std::move( info._data )),
                        _lastEdited( std::move( info._lastEdited ) )
                        {}

                        FileInfo( FileInfo& copy ) = default;
                        */
                    };

                    std::mutex                             _lockTracked;   ///< Protect access to tracked map.
                    std::map< key, std::vector<FileInfo> > _tracked;
                    std::vector<HANDLE>                    _waitHandle;
                    bool                                   _updateHandleList;
            };
            using TrackerThreadSPtr = std::shared_ptr<TrackerThread>;

            FileTracker();
            ~FileTracker();

            void registerResource( ResourceSPtr resource );
            void unregisterResource( ResourceSPtr resource );

            //------------------------------------------------------------------------
            /// \brief  Start tracking system.
            /// After this operation, we have guaranty that threading has make first loop.
            /// 
            void startTracking();
            void stopTracking();

            Signal<Resource&>& signalFileChanged()
            {
                MouCa::preCondition(_thread != nullptr);
                return _thread->signalFileChanged();
            }

        private:
            Signal<Resource&>   _onFileChanged;
            TrackerThreadSPtr   _thread;
    };

    using FileTrackerSPtr = std::shared_ptr<FileTracker>;

}
