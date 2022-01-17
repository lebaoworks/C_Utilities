#pragma once
#include <Windows.h>

#include <string>
#include <map>

struct USER_INFO {
	std::wstring Name;
	std::wstring DomainName;
	std::wstring SID;
};

DWORD GetFileOwner(std::wstring path, USER_INFO& info);
DWORD GetFileOwnerEx(std::map<std::wstring, USER_INFO>& info);