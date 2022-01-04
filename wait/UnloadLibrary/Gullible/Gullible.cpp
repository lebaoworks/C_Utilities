// gullible.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <iostream>
#include <string>

using namespace std;


HMODULE test_loadlib(const char* name)
{
    HMODULE hModule = LoadLibraryA(name);
    if (hModule == NULL)
    {
        cout << "[!] Load library " << name << " error << " << endl;
        ExitProcess(1);
    }
    cout << "Load " << name << " " << hModule << endl;
    return hModule;
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

DWORD WINAPI unload_library_func(__in  LPVOID lpParameter)
{
    FreeLibraryAndExitThread((HMODULE)lpParameter, 0);
}


int main()
{
    HMODULE hSuspect = test_loadlib_as_datafile("SuspectLibrary.dll");
    test_loadlib("GullibleLibrary.dll");
    Sleep(2000);
    while (true)
    {
        printf("[PID: %d] Main is here, unload: \n", GetCurrentProcessId());
        //CreateThread(NULL, 0, unload_library_func, (LPVOID)hSuspect, 0, NULL);
        Sleep(2000);
    }
    return 0;
}
