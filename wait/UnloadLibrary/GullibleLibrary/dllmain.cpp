// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <iostream>
using namespace std;

void test_loadlib(const char* name)
{
    HMODULE hModule = LoadLibraryA(name);
    if (hModule == NULL)
    {
        cout << "[!] Load library " << name << " error << " << endl;
        ExitProcess(1);
    }
    cout << "Load " << name << " " << hModule << endl;
}

HMODULE test_loadlib_as_datafile(const char* name)
{
    HMODULE hModule = LoadLibraryExA(name, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule == NULL)
    {
        cout << "[!] Load library " << name << " error << " << GetLastError() << endl;
        ExitProcess(1);
    }
    cout << "Load " << name << " " << hModule << endl;
    return hModule;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        test_loadlib("SuspectLibrary.dll");
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

