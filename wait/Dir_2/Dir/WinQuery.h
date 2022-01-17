#pragma once

#include <Windows.h>

#include <string>
#include <vector>

#include "WinSec.h"

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
};
DWORD QueryDirectory(std::wstring path, std::vector<FILE_INFO>& files);