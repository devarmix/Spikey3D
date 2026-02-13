#pragma once

#if SPIKEY_PLATFORM_WIN32

#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NODRAWTEXT
#define NOCTLMGR
#define NOFLATSBAPIS

#include <Windows.h>

#undef MemoryBarrier
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef CreateDirectory
#undef GetComputerName
#undef GetUserName
#undef MessageBox
#undef GetCommandLine
#undef CreateWindow
#undef CreateProcess
#undef SetWindowText
#undef DrawText
#undef CreateFont
#undef IsMinimized
#undef IsMaximized
#undef LoadIcon
#undef InterlockedOr
#undef InterlockedAnd
#undef InterlockedExchange
#undef InterlockedCompareExchange
#undef InterlockedIncrement
#undef InterlockedDecrement
#undef InterlockedAdd
#undef GetObject
#undef GetClassName
#undef GetMessage
#undef CreateMutex
#undef DrawState
#undef LoadLibrary
#undef Yield
#undef GetEnvironmentVariable
#undef SetEnvironmentVariable

#endif