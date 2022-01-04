// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <thread>

#include <winternl.h>                   //PROCESS_BASIC_INFORMATION

// warning C4996: 'GetVersionExW': was declared deprecated
#pragma warning (disable : 4996)
bool IsWindows8OrGreater()
{
    OSVERSIONINFO ovi = { 0 };
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&ovi);

    if ((ovi.dwMajorVersion == 6 && ovi.dwMinorVersion >= 2) || ovi.dwMajorVersion > 6)
        return true;

    return false;
} //IsWindows8OrGreater
#pragma warning (default : 4996)



bool ReadMem(void* addr, void* buf, int size)
{
    BOOL b = ReadProcessMemory(GetCurrentProcess(), addr, buf, size, nullptr);
    return b != FALSE;
}

#ifdef _WIN64
#define BITNESS 1
#else
#define BITNESS 0
#endif

typedef NTSTATUS(NTAPI* pfuncNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

//
//  Queries for .dll module load count, returns 0 if fails.
//
int GetModuleLoadCount(HMODULE hDll)
{
    // Not supported by earlier versions of windows.
    if (!IsWindows8OrGreater())
        return 0;

    PROCESS_BASIC_INFORMATION pbi = { 0 };

    HMODULE hNtDll = LoadLibraryA("ntdll.dll");
    if (!hNtDll)
        return 0;

    pfuncNtQueryInformationProcess pNtQueryInformationProcess = (pfuncNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");
    bool b = pNtQueryInformationProcess != nullptr;
    if (b) b = NT_SUCCESS(pNtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &pbi, sizeof(pbi), nullptr));
    FreeLibrary(hNtDll);

    if (!b)
        return 0;

    char* LdrDataOffset = (char*)(pbi.PebBaseAddress) + offsetof(PEB, Ldr);
    char* addr;
    PEB_LDR_DATA LdrData;

    if (!ReadMem(LdrDataOffset, &addr, sizeof(void*)) || !ReadMem(addr, &LdrData, sizeof(LdrData)))
        return 0;

    LIST_ENTRY* head = LdrData.InMemoryOrderModuleList.Flink;
    LIST_ENTRY* next = head;

    do {
        LDR_DATA_TABLE_ENTRY LdrEntry;
        LDR_DATA_TABLE_ENTRY* pLdrEntry = CONTAINING_RECORD(head, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (!ReadMem(pLdrEntry, &LdrEntry, sizeof(LdrEntry)))
            return 0;

        if (LdrEntry.DllBase == (void*)hDll)
        {
            //  
            //  http://www.geoffchappell.com/studies/windows/win32/ntdll/structs/ldr_data_table_entry.htm
            //
            int offDdagNode = (0x14 - BITNESS) * sizeof(void*);   // See offset on LDR_DDAG_NODE *DdagNode;

            ULONG count = 0;
            char* addrDdagNode = ((char*)pLdrEntry) + offDdagNode;

            //
            //  http://www.geoffchappell.com/studies/windows/win32/ntdll/structs/ldr_ddag_node.htm
            //  See offset on ULONG LoadCount;
            //
            if (!ReadMem(addrDdagNode, &addr, sizeof(void*)) || !ReadMem(addr + 3 * sizeof(void*), &count, sizeof(count)))
                return 0;
            return (int)count;
        } //if

        head = LdrEntry.InMemoryOrderLinks.Flink;
    } while (head != next);

    return 0;
} //GetModuleLoadCount

DWORD WINAPI suspect_func(__in  LPVOID lpParameter)
{
    while (true)
    {
        printf("[PID: %d][TID: %d][LoadCount: %d] Do I look suspect?\n", GetCurrentProcessId(), GetCurrentThreadId(), GetModuleLoadCount((HMODULE)lpParameter));
        Sleep(1000);
    }
    return 0;
}

int pa_count = 0;
int ta_count = 0;
int pd_count = 0;
int td_count = 0;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        printf("[PID: %d][TID: %d] PROCESS_ATTACH -> %d\n", GetCurrentProcessId(), GetCurrentThreadId(), ++pa_count);
        CreateThread(NULL, 0, suspect_func, (LPVOID) hModule, 0, NULL);
        break; }
    case DLL_PROCESS_DETACH:
        printf("[PID: %d][TID: %d] PROCESS_DETACH -> %d\n", GetCurrentProcessId(), GetCurrentThreadId(), ++td_count);
        break;
    case DLL_THREAD_ATTACH:
        printf("[PID: %d][TID: %d] THREAD_ATTACH -> %d\n", GetCurrentProcessId(), GetCurrentThreadId(), ++ta_count);
        break;
    case DLL_THREAD_DETACH:
        printf("[PID: %d][TID: %d] THREAD_DETACH -> %d\n", GetCurrentProcessId(), GetCurrentThreadId(), ++pd_count);
        break;
    }
    return TRUE;
}

