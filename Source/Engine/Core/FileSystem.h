#pragma once

#if SPIKEY_PLATFORM_WIN32
#include <Platform/Windows/WinFileSystem.h>
typedef Spikey::WinFile File;
#else
#error Unsupported Platform!
#endif