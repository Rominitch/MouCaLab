/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    class Process
    {
        public:
            enum class State : uint8_t
            {
                NotExecuted,
                Running
            };
            using Arguments = std::vector<Core::StringOS>;

            Process(const Core::StringOS& executable, const bool inheritHandle=false);
            ~Process() = default;

            void execute(const Arguments& arguments = Arguments());

            bool isRunning() const
            {
                return _state == State::Running;
            }

            bool waitForFinish(const int timeout = 0) noexcept;

            int getReturnCode() const
            {
                return _returnCode;
            }

            Core::String readStdOutput() const noexcept;
            Core::String readStdError() const noexcept;

        private:
            const bool   _inheritHandle;
            State        _state;
            int          _returnCode;
            Core::StringOS _executable;

#ifdef MOUCA_OS_WINDOWS
            class ProcessPipe final
            {
                public:
                    ProcessPipe();
                    ~ProcessPipe();

                    void readyToRead();

                    HANDLE getHandleRead() const  { return _handleRead; }
                    HANDLE getHandleWrite() const { return _handleWrite; }

                private:
                    HANDLE _handleRead;
                    HANDLE _handleWrite;
            };

            PROCESS_INFORMATION _pi;
            ProcessPipe         _stdOut;
            ProcessPipe         _stdErr;

            Core::String readStdGeneric(const ProcessPipe& pipe) const noexcept;
#else
#   error  "Need to migrate !!!"
#endif
            
    };
}