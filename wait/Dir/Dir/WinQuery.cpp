#include "WinQuery.h"

#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")
#include <Shlwapi.h>
#include <fileapi.h>

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
    WIN32_FIND_STREAM_DATA fsd;
    HANDLE hFind = FindFirstFile((path + LR"(\*)").c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        return GetLastError();
    do
    {
        FILE_INFO x = {};
        x.Name = ffd.cFileName;
        if (x.Name.compare(L".") == 0 ||
            x.Name.compare(L"..") == 0)
            continue;
        x.FullName = ((path.back() == L'\\') ? path : path + L"\\") + x.Name;
        x.DosName = ffd.cAlternateFileName;
        x.Attributes = ffd.dwFileAttributes;
        x.Size.LowPart = (x.Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : ffd.nFileSizeLow;
        x.Size.HighPart = (x.Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : ffd.nFileSizeHigh;
        x.CreateTime = ffd.ftCreationTime;
        x.ModifyTime = ffd.ftLastWriteTime;
        x.AccessTime = ffd.ftLastAccessTime;
        GetFileOwner(x.FullName, x.Owner);
        x.Streams = vector<STREAM_INFO>();
        // Find alternate streams
        if ((x.Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            HANDLE hFindStreams = FindFirstStreamW(x.FullName.c_str(), FindStreamInfoStandard, &fsd, 0);
            if (hFindStreams != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (wcscmp(fsd.cStreamName, L"::$DATA") != 0)
                        x.Streams.push_back(STREAM_INFO{ fsd.cStreamName, fsd.StreamSize });
                } while (FindNextStreamW(hFindStreams, &fsd));
                FindClose(hFindStreams);
            }
        }

        files.push_back(x);
    } while (FindNextFile(hFind, &ffd) != 0);

    dwRet = GetLastError();
    dwRet = (dwRet == ERROR_NO_MORE_FILES)? 0 : dwRet;

    FindClose(hFind);
    return dwRet;
}
