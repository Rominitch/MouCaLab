/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

/// OS
#if defined _WIN32 || defined _WIN64
#   define MOUCA_OS_WINDOWS
#elif defined(__linux__)
#   define MOUCA_OS_LINUX
#elif defined(__APPLE__)
#   define MOUCA_OS_APPLE
#elif defined(__ANDROID__)
#   define MOUCA_OS_ANDROID
#else
#   error Undefined OS.
#endif
