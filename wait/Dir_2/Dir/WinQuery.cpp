#include "WinQuery.h"

#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")
#include <Shlwapi.h>

#include <vector>
#include <string>
using namespace std;

DWORD QueryDirectory(wstring path, vector<FILE_INFO>& files)
{
    files.clear();

    if (path.length() == 0 || PathIsRelative(path.c_str()))
        return ERROR_INVALID_PARAMETER;

    DWORD dwRet;
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile((path + LR"(\*)").c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        return GetLastError();
    do
    {
        FILE_INFO x = {};
        x.Attributes = ffd.dwFileAttributes;
        x.Name = ffd.cFileName;
        x.FullName = ((path.back() == L'\\') ? path : path + L"\\") + x.Name;
        x.Size.LowPart = (x.Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : ffd.nFileSizeLow;
        x.Size.HighPart = (x.Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : ffd.nFileSizeHigh;
        x.CreateTime = ffd.ftCreationTime;
        x.ModifyTime = ffd.ftLastWriteTime;
        x.AccessTime = ffd.ftLastAccessTime;
        dwRet = GetFileOwner(x.FullName, x.Owner);
        files.push_back(x);
    } while (FindNextFile(hFind, &ffd) != 0);

    dwRet = GetLastError();
    dwRet = (dwRet == ERROR_NO_MORE_FILES)? 0 : dwRet;

    FindClose(hFind);
    return dwRet;
}
