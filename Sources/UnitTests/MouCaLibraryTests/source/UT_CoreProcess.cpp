#include "Dependencies.h"

#include <LibCore/include/CoreProcess.h>

TEST(CoreProcess, callIpConfig)
{
    Core::Process command(L"C:\\Windows\\System32\\ipconfig.exe", true);

    ASSERT_NO_THROW(command.execute());
    
    // Execute command
    ASSERT_TRUE(command.isRunning());
    ASSERT_NO_THROW(command.waitForFinish());
    ASSERT_FALSE(command.isRunning());

    // Read output after finish execution
    Core::String output;
    ASSERT_NO_THROW(output = command.readStdOutput());
    ASSERT_FALSE(output.empty());
    Core::String cerr;
    ASSERT_NO_THROW(cerr = command.readStdError());
    ASSERT_TRUE(cerr.empty());

    ASSERT_EQ(0, command.getReturnCode());    
}

TEST(CoreProcess, callArguments)
{
    Core::Process command(L"C:\\Windows\\System32\\ipconfig.exe", true);
    
    Core::Process::Arguments args;
    args.push_back(Core::StringOS(L"/all"));
    ASSERT_NO_THROW(command.execute(args));

    // Execute command
    ASSERT_TRUE(command.isRunning());
    ASSERT_NO_THROW(command.waitForFinish());
    ASSERT_FALSE(command.isRunning());

    // Read output after finish execution
    Core::String output;
    ASSERT_NO_THROW(output = command.readStdOutput());
    ASSERT_FALSE(output.empty());
    Core::String cerr;
    ASSERT_NO_THROW(cerr = command.readStdError());
    ASSERT_TRUE(cerr.empty());

    ASSERT_EQ(0, command.getReturnCode());
}

TEST(CoreProcess, badReturn)
{
    Core::Process command(L"C:\\Windows\\System32\\ipconfig.exe", true);

    Core::Process::Arguments args;
    args.push_back(Core::StringOS(L"marcel"));
    ASSERT_NO_THROW(command.execute(args));

    // Execute command
    ASSERT_TRUE(command.isRunning());
    ASSERT_NO_THROW(command.waitForFinish());
    ASSERT_FALSE(command.isRunning());

    ASSERT_NE(0, command.getReturnCode());
}