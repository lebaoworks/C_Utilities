#pragma once

#include <Windows.h>
#include <string>
#include <map>

struct MFT_FILE_INFO {
	std::wstring Name;
	std::wstring DosName;
	FILETIME CreateTime;
	FILETIME ModifyTime;
	FILETIME AccessTime;
};
void QueryMFTPath(std::wstring abs_path, std::map<std::wstring, MFT_FILE_INFO>& ret);