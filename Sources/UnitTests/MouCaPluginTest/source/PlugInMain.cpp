#include "Dependancies.h"

#include "include/PlugInMain.h"

// Declare my template to build entry point into DLL
DECLARE_PLUG_IN(PlugInTest);

void PlugInTest::initialize()
{
    std::cout << "Welcome to plugin" << std::endl;
}

void PlugInTest::release()
{
    std::cout << "See you in space cowboy !" << std::endl;
}