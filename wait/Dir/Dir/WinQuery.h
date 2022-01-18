#pragma once

#include <Windows.h>

#include <string>
#include <vector>

#include "WinSec.h"

struct STREAM_INFO
{
    std::wstring Name;
    LARGE_INTEGER Size;
};
struct FILE_INFO {
    std::wstring Name;
    std::wstring FullName;
    std::wstring DosName;
    USER_INFO Owner;
    LARGE_INTEGER Size;
    DWORD Attributes;
    FILETIME CreateTime;
    FILETIME ModifyTime;
    FILETIME AccessTime;
    FILETIME MFTCreateTime;
    FILETIME MFTModifyTime;
    FILETIME MFTAccessTime;
    std::vector<STREAM_INFO> Streams;
};
DWORD QueryDirectory(std::wstring path, std::vector<FILE_INFO>& files);