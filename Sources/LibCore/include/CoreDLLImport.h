#pragma once

#ifdef  MOUCACORE_ENGINE_EXPORTS
#	define MOUCACORE_ENGINE_API __declspec(dllexport)
#else
#	define MOUCACORE_ENGINE_API __declspec(dllimport)
#endif