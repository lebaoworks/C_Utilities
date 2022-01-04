#define USE_WINAPI
#define DEBUG

/*
*   INCLUDE SELF
*/
#include "DllUtils.h"

/*
*   INCLUDE FOR WINDOWS
*/
#include <Windows.h>
#include <TlHelp32.h>
#include <psapi.h>

/*
*   INCLUDE C++
*/
#include <string>
#include <vector>
using namespace std;

/*
*   INCLUDE ADDITION
*/
#include "kernel_ntdll.h"
#include "kernel_ntdll32.h"
#include "dllhelper.h"

/*
*   UNEXPORT
*/
#ifdef DEBUG
#define log_info(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__);
#define wlog_info(fmt, ...) fwprintf(stderr, fmt, __VA_ARGS__);
#else
    #define log_info(fmt, ...) ;
    #define wlog_info(fmt, ...) ;
#endif

class NTDLL_API {
    DllHelper _dll{ L"ntdll.dll" };
public:
    decltype(NtQueryInformationProcess)* _NtQueryInformationProcess = _dll["NtQueryInformationProcess"];
    decltype(NtQueryInformationThread)*  _NtQueryInformationThread  = _dll["NtQueryInformationThread"];
};
NTDLL_API NTDLL;

BOOL IsEmulated(HANDLE hProcess)
{
    BOOL isWOW64;
    if (!IsWow64Process(hProcess, &isWOW64))
        return TRUE;
    return isWOW64;
}

/*
*   IMPLEMENT
*/
#ifdef USE_WINAPI
DWORD GetModules(DWORD pid, vector<MODULE_INFO>& list)
{
    list.clear();

    // Check if this process run as 32-bit mode in windows x64
    if (IsEmulated(GetCurrentProcess()))
        return -1;

    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, pid);
    if (h == NULL)
        return GetLastError();

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(entry);
    if (!Module32First(h, &entry))
    {
        CloseHandle(h);
        return GetLastError();
    }
    do
    {
        MODULE_INFO x;
        x.FullDllName = entry.szExePath;
        x.BaseDllName = entry.szModule;
        x.DllBase = entry.modBaseAddr;
        x.SizeOfImage = entry.modBaseSize;
        x.LoadCount = entry.ProccntUsage;
        list.push_back(x);
    } while (Module32Next(h, &entry));

    CloseHandle(h);
    return 0;
}
#else
DWORD GetWOW64Modules(DWORD pid, vector<MODULE_INFO>& list)
{
    // Check if this process run as 32-bit mode in windows x64
    if (IsEmulated(GetCurrentProcess()))
        return -1;

    DWORD iStatus;
    SIZE_T size;
    log_info("[*] OpenProcess... ");
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
    if (hProcess == NULL)
    {
        iStatus = GetLastError();
        log_info("=> Error %d\n", iStatus);
        return iStatus;
    }
    log_info("=> Done\n");

    // Query PEB address
    log_info("[*] Query PEB addr... ");
    LPVOID PebBaseAddress;
    iStatus = NTDLL._NtQueryInformationProcess(hProcess, ProcessWow64Information, &PebBaseAddress, sizeof(PebBaseAddress), (ULONG*)&size);
    if (iStatus != 0)
    {
        CloseHandle(hProcess);
        return iStatus;
    }
    log_info("=> %p\n", PebBaseAddress);

    // Read PEB
    log_info("[*] Read PEB... ");
    PEB32 peb;
    if (!ReadProcessMemory(hProcess, PebBaseAddress, &peb, sizeof(peb), &size))
        goto Error;
    log_info("=> Done\n");

    // Read LDR to get address of double-linked list to modules info
    log_info("[*] Read LDR in (%p)... ", peb.Ldr);
    PEB_LDR_DATA32 ldr;
    if (!ReadProcessMemory(hProcess, (LPCVOID)peb.Ldr, &ldr, sizeof(ldr), &size))
        goto Error;
    log_info("=> Done\n");

    // Walk through double-linkeed list to read modules info
    log_info("[*] Walk LDR list...\n");
    LDR_DATA_TABLE_ENTRY32 module;
    for (
        LIST_ENTRY32* entry = (LIST_ENTRY32*) ldr.InLoadOrderModuleList.Flink;
        entry != (PVOID)((UINT64)peb.Ldr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList)); // stop when got back to the first element
        entry = (LIST_ENTRY32*) module.InLoadOrderLinks.Flink)
    {
        // Read module info
        if (!ReadProcessMemory(hProcess, entry, &module, sizeof(module), &size))
            goto Error;

        // Read module name
        WCHAR* full_name = (WCHAR*)calloc(module.FullDllName.Length + 1, sizeof(WCHAR));
        if (full_name == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }
        WCHAR* base_name = (WCHAR*)calloc(module.BaseDllName.Length + 1, sizeof(WCHAR));
        if (base_name == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }

        if (!ReadProcessMemory(hProcess, (void*) module.FullDllName.Buffer, full_name, module.FullDllName.Length, &size) ||
            !ReadProcessMemory(hProcess, (void*) module.BaseDllName.Buffer, base_name, module.BaseDllName.Length, &size))
        {
            free(full_name);
            free(base_name);
            goto Error;
        }
        wlog_info(L"\t[+] Module Base: %p\tSize: %lu\tModule Name: %s\n", module.DllBase, module.SizeOfImage, full_name);

        MODULE_INFO info = { wstring(full_name), wstring(base_name), (void*) module.DllBase, module.SizeOfImage };
        list.push_back(info);

        free(full_name);
        free(base_name);
    }

    CloseHandle(hProcess);

Error:
    iStatus = GetLastError();
    log_info("=> Error %d\n", iStatus);
    CloseHandle(hProcess);
    list.clear();
    return iStatus;
}
DWORD GetModules(DWORD pid, vector<MODULE_INFO>& list)
{
    list.clear();

    // Check if this process run as 32-bit mode in windows x64
    if (IsEmulated(GetCurrentProcess()))
        return -1;

    DWORD iStatus;
    SIZE_T size;
    log_info("[*] OpenProcess... ");
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
    if (hProcess == NULL)
    {
        iStatus = GetLastError();
        log_info("=> Error %d\n", iStatus);
        return iStatus;
    }
    log_info("=> Done\n");

    // Query PEB address
    log_info("[*] Query PEB addr... ");
    PROCESS_BASIC_INFORMATION pbi;
    iStatus = NTDLL._NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), (ULONG*)&size);
    if (iStatus != 0)
    {
        CloseHandle(hProcess);
        return iStatus;
    }
    log_info("=> %p\n", pbi.PebBaseAddress);

    // Read PEB
    log_info("[*] Read PEB... ");
    PEB peb;
    if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &size))
        goto Error;
    log_info("=> Done\n");

    // Read LDR to get address of double-linked list to modules info
    log_info("[*] Read LDR in (%p)... ", peb.Ldr);
    PEB_LDR_DATA ldr;
    if (!ReadProcessMemory(hProcess, (LPCVOID)peb.Ldr, &ldr, sizeof(ldr), &size))
        goto Error;
    log_info("=> Done\n");

    // Walk through double-linkeed list to read modules info
    log_info("[*] Walk LDR list...\n");
    LDR_DATA_TABLE_ENTRY module;
    LDR_DDAG_NODE ddag_node;
    for (
        PLIST_ENTRY entry = ldr.InLoadOrderModuleList.Flink;
        entry != (PVOID)((UINT64)peb.Ldr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList)); // stop when got back to the first element
        entry = module.InLoadOrderLinks.Flink)
    {
        // Read module info
        if (!ReadProcessMemory(hProcess, entry, &module, sizeof(module), &size))
            goto Error;

        // Read module name
        WCHAR* full_name = (WCHAR*)calloc(module.FullDllName.Length + 1, sizeof(WCHAR));
        if (full_name == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }
        WCHAR* base_name = (WCHAR*)calloc(module.BaseDllName.Length + 1, sizeof(WCHAR));
        if (base_name == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }

        if (!ReadProcessMemory(hProcess, module.FullDllName.Buffer, full_name, module.FullDllName.Length, &size) ||
            !ReadProcessMemory(hProcess, module.BaseDllName.Buffer, base_name, module.BaseDllName.Length, &size))
        {
            free(full_name);
            free(base_name);
            goto Error;
        }
        
        // Read module load count
        if (!ReadProcessMemory(hProcess, module.DdagNode, &ddag_node, sizeof(ddag_node), &size))
            goto Error;


        wlog_info(L"\t[+] Module Base: %p\tSize: %lu\tModule Name: %s\tLoadCount: %d\n", module.DllBase, module.SizeOfImage, full_name, ddag_node.LoadCount);

        MODULE_INFO info = {wstring(full_name), wstring(base_name), module.DllBase, module.SizeOfImage, ddag_node.LoadCount};
        list.push_back(info);

        free(full_name);
        free(base_name);
    }
    if (IsEmulated(hProcess))
    {
        vector<MODULE_INFO> wow64_modules;
        iStatus = GetWOW64Modules(pid, wow64_modules);
        if (iStatus != 0)
        {
            SetLastError(iStatus);
            goto Error;
        }
        list.insert(list.end(), wow64_modules.begin(), wow64_modules.end());
    }

    CloseHandle(hProcess);
    return 0;

Error:
    iStatus = GetLastError();
    log_info("=> Error %d\n", iStatus);
    CloseHandle(hProcess);
    list.clear();
    return iStatus;
}
#endif

DWORD GetThreads(DWORD pid, vector<THREAD_INFO>& list)
{
    list.clear();

    // Check if this process run as 32-bit mode in windows x64
    if (IsEmulated(GetCurrentProcess()))
        return -1;

    // Snap to get list of threads
    log_info("[*] Snapshot... ");
    vector<DWORD> tids;
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return GetLastError();

    // Walk through snapshot to get threads id
    log_info("[*] Walk through snapshot... ");
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    if (!Thread32First(hThreadSnap, &te32))
    {
        CloseHandle(hThreadSnap);
        return GetLastError();
    }
    do
    {
        if (te32.th32OwnerProcessID == pid)
            tids.push_back(te32.th32ThreadID);
    } while (Thread32Next(hThreadSnap, &te32));
    CloseHandle(hThreadSnap);

    // Query threads' start_address
    log_info("[*] Query threads' start_address...\n");
    for (int i = 0; i < tids.size(); i++)
    {
        // Query TEB address
        HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, false, tids[i]);
        if (hThread == NULL)
            return GetLastError();

        PVOID start_address;
        DWORD outLength = 0;
        NTSTATUS iStatus = NTDLL._NtQueryInformationThread(
            hThread,
            ThreadQuerySetWin32StartAddress,
            &start_address, sizeof(start_address), &outLength
        );
        if (iStatus != 0)
            return iStatus;

        THREAD_INFO info = { tids[i], pid, start_address };
        list.push_back(info);
        CloseHandle(hThread);
    }
    return 0;
}
