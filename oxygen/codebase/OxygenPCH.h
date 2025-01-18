#pragma once

#if defined(_WIN32) && defined(_WIN64) && defined(_MSC_VER)
#include "Platform/PlatformWin64/PrecompiledHeaders/PCH.h"
#else
#error "Unsupported platform"
#endif

#include "Platform/InternalPCHBase.h"