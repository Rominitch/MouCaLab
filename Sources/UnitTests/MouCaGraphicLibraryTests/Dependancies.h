#pragma once

//Common include
#include <Cpp.h>

//Boost
#include <Boost.h>

//SDK
#include <gTest.h>

#include <OpenVR.h>
#include <GLFW.h>
#include <Vulkan.h>
#include <imgui1.h>

#include <FreetypePackage.h>

//MouCa Common
#include <LibRT/RTGlobalDependancies.h>

#include <MouCaCore/MouCaCoreImport.h>

#define RT_AVAILABLE
#include "../include/gMouCaTest.h"

#ifdef  MOUCAENGINE
#	ifdef  MOUCAENGINE_EXPORTS 
#	define MOUCAENGINE_API __declspec(dllexport)
#	else
#	define MOUCAENGINE_API __declspec(dllimport)
#	endif
#else
#define MOUCAENGINE_API
#endif

//#define VULKAN_USER