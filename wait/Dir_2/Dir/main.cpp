#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")
#include <Shlwapi.h>

#include <string>
#include <vector>
using namespace std;

#include "WinSec.h"


struct FILE_INFO {
    wstring Name;
    wstring FullName;
    wstring DosName;
    wstring Owner;
    LARGE_INTEGER Size;
    DWORD Attributes;
};
BOOL dir(wstring path, vector<FILE_INFO>& files)
{
    files.clear();

    if (path.length() == 0 || PathIsRelative(path.c_str()))
        return FALSE;

    DWORD dwRet;
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile((path + LR"(\*)").c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("FindFirstFile fail %d\n", GetLastError());
        return TRUE;
    }

    // List all the files in the directory with some info about them.
    do
    {
        wstring owner, domain;
        FILE_INFO x = {};
        x.Attributes = ffd.dwFileAttributes;
        x.Name = ffd.cFileName;
        x.FullName = ((path.back() == L'\\') ? path : path + L"\\") + x.Name;
        x.Size.LowPart = (x.Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0:ffd.nFileSizeLow;
        x.Size.HighPart = (x.Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0:ffd.nFileSizeHigh;
        dwRet = GetFileOwner(x.FullName, domain, owner);
        if (dwRet == ERROR_NONE_MAPPED)
            x.Owner = L"<sid: " + owner + L">";
        else if (dwRet != 0)
            x.Owner = L"<Unknown>";
        else
            x.Owner = domain + L"\\" + owner;
        files.push_back(x);
    } while (FindNextFile(hFind, &ffd) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        wprintf(TEXT("FindFirstFile"));
    }

    FindClose(hFind);
    return dwError == 0;
}

#include <iostream>
int main()
{
    vector<FILE_INFO> files;
    //dir(LR"(C:\Program Files\WindowsApps)", files);
    dir(LR"(D:\Desktop)", files);

    map<wstring, USER_INFO> owner;
    for (int i = 0; i < files.size(); i++)
    {
        //wprintf(L"%s - %lld\n\t%s\n", files[i].Name.c_str(), files[i].Size.QuadPart, files[i].Owner.c_str());
        owner[files[i].FullName] = USER_INFO{};
    }
    GetFileOwnerEx(owner);
    for (map<wstring, USER_INFO>::iterator i = owner.begin(); i != owner.end(); i++)
        wcout << i->first << L"\n\t" << i->second.Name << L"\n\t" << i->second.SID << endl;
}