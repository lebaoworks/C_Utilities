#pragma once

//#define USE_WINAPI

#include <Windows.h>

#include <string>
#include <vector>

struct MODULE_INFO {
    std::wstring FullDllName;
    std::wstring BaseDllName;
    PVOID DllBase;
    ULONG SizeOfImage;
    DWORD LoadCount;
};
DWORD GetModules(DWORD pid, std::vector<MODULE_INFO>& list);

struct THREAD_INFO {
    DWORD tid;
    DWORD pid;
    PVOID StartAddress;
};
DWORD GetThreads(DWORD pid, std::vector<THREAD_INFO>& list);