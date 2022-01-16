#pragma once
#include <Windows.h>

#include <string>

struct USER_INFO
{
	std::wstring Name;
	std::wstring DomainName;
	std::wstring SID;
};

DWORD GetFileOwner(std::wstring path, std::wstring& domainName, std::wstring& userName);

#include <map>
DWORD GetFileOwnerEx(std::map<std::wstring, USER_INFO>& info);