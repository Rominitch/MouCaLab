/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibCore/include/CoreException.h"
#include "LibCore/include/CoreProcess.h"
#include "LibCore/include/CoreString.h"

namespace Core
{

Process::ProcessPipe::ProcessPipe() :
_handleRead(INVALID_HANDLE_VALUE), _handleWrite(INVALID_HANDLE_VALUE)
{
    SECURITY_ATTRIBUTES security_attributes
    {
        sizeof(SECURITY_ATTRIBUTES),
        nullptr,
        TRUE
    };

    // Build Pipe
    if(!CreatePipe(&_handleRead, &_handleWrite, &security_attributes, 0))
    {
        BT_THROW_ERROR(u8"Basic", u8"ProcessError");
    }
    if(!SetHandleInformation(_handleRead, HANDLE_FLAG_INHERIT, 0))
    {
        BT_THROW_ERROR(u8"Basic", u8"ProcessError");
    }

    MOUCA_POST_CONDITION(_handleRead  != INVALID_HANDLE_VALUE);
    MOUCA_POST_CONDITION(_handleWrite != INVALID_HANDLE_VALUE);
}

Process::ProcessPipe::~ProcessPipe()
{
    if(_handleRead != INVALID_HANDLE_VALUE)
    { 
        CloseHandle(_handleRead);
    }
        
    if(_handleWrite != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_handleWrite);
    }
}

void Process::ProcessPipe::readyToRead()
{
    MOUCA_POST_CONDITION(_handleRead  != INVALID_HANDLE_VALUE);
    MOUCA_POST_CONDITION(_handleWrite != INVALID_HANDLE_VALUE);

    // Close handle write to avoid lock
    CloseHandle(_handleWrite);
    _handleWrite = INVALID_HANDLE_VALUE;
}

Process::Process(const Core::StringOS& executable, const bool inheritHandle) :
_executable(executable), _state(State::NotExecuted), _returnCode(0), _inheritHandle(inheritHandle)
{
    MOUCA_PRE_CONDITION(std::filesystem::exists(executable)); // Dev Issue: prefer call program from full absolute path.
}

void Process::execute(const std::vector<Core::StringOS>& arguments)
{
    MOUCA_PRE_CONDITION(_state == State::NotExecuted); //DEV Issue: no re-entrance (maybe more strong ?)

    // Build command
    const Core::StringOS delimiter(L" ");
    Core::StringOS command = _executable;

    // Add to command all arguments
    if(!arguments.empty())
    {
        command += delimiter;

        auto itArg = arguments.cbegin();
        auto itFinalArg = itArg;
        std::advance(itFinalArg, arguments.size() - 1);
        while(itArg != arguments.cend())
        {
            command += *itArg;
            if(itArg != itFinalArg)
            {
                command += delimiter;
            }
            ++itArg;
        }
    }

    STARTUPINFO si;    
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb         = sizeof(STARTUPINFO);
    si.hStdOutput = _stdOut.getHandleWrite();
    si.hStdError  = _stdErr.getHandleWrite();
    si.dwFlags   |= STARTF_USESTDHANDLES;

    ZeroMemory(&_pi, sizeof(PROCESS_INFORMATION));

    if(CreateProcess(
        NULL,                           // lpApplicationName
        &command[0],                    // lpCommandLine
        NULL,                           // lpProcessAttributes
        NULL,                           // lpThreadAttributes
        _inheritHandle ? TRUE : FALSE,  // bInheritHandles
        0,                              // dwCreationFlags
        NULL,                           // lpEnvironment
        NULL,                           // lpCurrentDirectory
        &si,                            // lpStartupInfo
        &_pi                            // lpProcessInformation
    ) == 0)
    {
        BT_THROW_ERROR(u8"Basic", u8"ProcessError");
    }

    _stdOut.readyToRead();
    _stdErr.readyToRead();

    _state = State::Running;
}

Core::String Process::readStdOutput() const noexcept
{
    return readStdGeneric(_stdOut);
}

Core::String Process::readStdError() const noexcept
{
    return readStdGeneric(_stdErr);
}

Core::String Process::readStdGeneric(const ProcessPipe& pipe) const noexcept
{
    MOUCA_PRE_CONDITION(pipe.getHandleRead() != INVALID_HANDLE_VALUE);
    
    Core::String output;
    if(_pi.hProcess != NULL)
    {
        DWORD n;
        
        Core::String local;
        local.resize(1500);
        BOOL success;
        do
        {
            success = ReadFile(pipe.getHandleRead(), local.data(), static_cast<DWORD>(local.size()), &n, nullptr);
            output.append( local, 0, n);
        }
        while(success && n != 0);
    }
    return output;
}

bool Process::waitForFinish(const int timeout) noexcept
{
    if( _pi.hProcess == INVALID_HANDLE_VALUE )
        return true;

    DWORD timeoutWait = timeout == 0 ? INFINITE : timeout;
    // Wait until timeout or done
    if(WaitForSingleObject(_pi.hProcess, timeoutWait) != WAIT_TIMEOUT)
    {
        DWORD exit_status_win;
        _returnCode = GetExitCodeProcess(_pi.hProcess, &exit_status_win) ? static_cast<int>(exit_status_win) : -1;

        // Delete all process info
        CloseHandle(_pi.hProcess);
        CloseHandle(_pi.hThread);
        _pi.hProcess = INVALID_HANDLE_VALUE;
        _pi.hThread  = INVALID_HANDLE_VALUE;

        // Now is the end
        _state = State::NotExecuted;

        return true;
    }
    return false;
}

}