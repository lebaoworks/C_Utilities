//
// DIR <absolute_path>
//		/A:[-]attributes		Displays files with specified attributes.
//			attributes	D  Directories		R  Read-only files
//						H  Hidden files		A  Files ready for archiving
//						S  System files		I  Not content indexed files
//						L  Reparse Points	O  Offline files
//						-  Prefix to exclude attribute
//		/L	Lower-case filename
//		/O:[-]sortorder			List by files in sorted order.
//			sortorder	N  By name (alphabetic)			S  By size (smallest first)
//						E  By extension (alphabetic)	D  By date/time (oldest first)
//						G  Group directories first
//						- Prefix to reverse order
//		/T:timefield			Choose which time field displayed
//			  timefield		C  Creation		M MFT Creation
//							A  Last Access	W  Last Written
//		/Q	Display the owner of the file.
//		/R	Display alternate data streams of the file.
//		/S	Displays files in specified directory and all subdirectories.
//		/X	This displays the short names generated for non-8dot3 file names.
//

#include <Windows.h>

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex>
using namespace std;

#include "subprocess.h"
#include "cppstring.h"
#include "mft.h"

#pragma comment(lib, "Shlwapi.lib")
#include <shlwapi.h>

struct FILE_INFO
{
	wstring Name;
	wstring DosName;
	wstring FullName;
	FILETIME CreateTime;		//Local time
	FILETIME ModifyTime;		//Local time
	FILETIME AccessTime;		//Local time
	ULONG Attributes;			//https://docs.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
	wstring Owner;				//DomainName + UserName
};
void QueryPWSDir(wstring path, map<wstring, vector<FILE_INFO>>& out)
{
	out.clear();

	// Check path
	if (path.length() == 0 || path.length()>=MAX_PATH || PathIsRelative(path.c_str()) || path.find_first_of(L" '\"") != wstring::npos)
		return;

	// Call powershell
	vector<wstring> lines;
	wstring cmd_line = L"powershell.exe -c"
		" \""
		"Get-ChildItem $Path -Force |"
		"Select"
		"   Name,"
		"   FullName,"
		"   Mode,"
		"   CreationTime,"
		"   LastWriteTime,"
		"   LastAccessTime,"
		"   @{Name='Owner';Expression={ (Get-Acl $_.FullName).Owner }} |"
		"Format-List"
		"\"";
	replace(cmd_line, wstring(L"$Path"), wstring(L"'") + path + L"'");
	RunCmdAndGetText(cmd_line, lines);

	FILE_INFO file_info;
	SYSTEMTIME x;
	wstring value;
	wregex time_exp = wregex(LR"((\d+)\/(\d+)\/(\d+)\s+(\d+):(\d+):(\d+)\s+([AP])M)");
	wsmatch match;

	for (int i = 0; i < lines.size(); i++)
	{
		trim(lines[i]);
		size_t delimiter_pos = lines[i].find_first_of(L":");
		if (delimiter_pos == wstring::npos)
			continue;

		value = (lines[i].length() > delimiter_pos + 1) ? lines[i].substr(delimiter_pos + 1, lines[i].length()) : L"";
		trim(value);

		if (lines[i].compare(0, 4, L"Name") == 0)
		{
			file_info.Name = value;
			file_info.Owner = file_info.FullName = file_info.DosName = L"";
			file_info.Attributes = 0;
			memset(&file_info.CreateTime, 0, sizeof(FILETIME));
			memset(&file_info.ModifyTime, 0, sizeof(FILETIME));
			memset(&file_info.AccessTime, 0, sizeof(FILETIME));
		}
		if (lines[i].compare(0, 8, L"FullName") == 0)
		{
			file_info.FullName = value;
		}
		else if (lines[i].compare(0,4, L"Mode") == 0)
		{ 
			for (int ii=0; ii<value.size(); ii++)
				switch (value[ii])
				{
				case L'l':
					file_info.Attributes |= FILE_ATTRIBUTE_REPARSE_POINT;
					break;
				case L'd':
					file_info.Attributes |= FILE_ATTRIBUTE_DIRECTORY;
					break;
				case L'a':
					file_info.Attributes |= FILE_ATTRIBUTE_ARCHIVE;
					break;
				case L'r':
					file_info.Attributes |= FILE_ATTRIBUTE_READONLY;
					break;
				case L'h':
					file_info.Attributes |= FILE_ATTRIBUTE_HIDDEN;
					break;
				case L's':
					file_info.Attributes |= FILE_ATTRIBUTE_SYSTEM;
					break;
				}
		}
		else if (lines[i].compare(0, 12, L"CreationTime") == 0 && regex_search(value, match, time_exp))
		{
			x.wYear = _wtoi(match[3].str().c_str());
			x.wMonth = _wtoi(match[1].str().c_str());
			x.wDay = _wtoi(match[2].str().c_str());
			x.wHour = _wtoi(match[4].str().c_str());
			x.wMinute = _wtoi(match[5].str().c_str());
			x.wSecond = _wtoi(match[6].str().c_str());
			x.wMilliseconds = 0;
			x.wDayOfWeek = 0;
			SystemTimeToFileTime(&x, &file_info.CreateTime);
		}
		else if (lines[i].compare(0, 13, L"LastWriteTime") == 0 && regex_search(value, match, time_exp))
		{
			x.wYear = _wtoi(match[3].str().c_str());
			x.wMonth = _wtoi(match[1].str().c_str());
			x.wDay = _wtoi(match[2].str().c_str());
			x.wHour = _wtoi(match[4].str().c_str());
			x.wMinute = _wtoi(match[5].str().c_str());
			x.wSecond = _wtoi(match[6].str().c_str());
			x.wMilliseconds = 0;
			x.wDayOfWeek = 0;
			SystemTimeToFileTime(&x, &file_info.ModifyTime);
		}
		else if (lines[i].compare(0, 14, L"LastAccessTime") == 0 && regex_search(value, match, time_exp))
		{
			x.wYear = _wtoi(match[3].str().c_str());
			x.wMonth = _wtoi(match[1].str().c_str());
			x.wDay = _wtoi(match[2].str().c_str());
			x.wHour = _wtoi(match[4].str().c_str());
			x.wMinute = _wtoi(match[5].str().c_str());
			x.wSecond = _wtoi(match[6].str().c_str());
			x.wMilliseconds = 0;
			x.wDayOfWeek = 0;
			SystemTimeToFileTime(&x, &file_info.AccessTime);
		}
		else if (lines[i].compare(0, 5, L"Owner") == 0)
		{
			file_info.Owner = value;
			if (file_info.FullName.length() > 0)
			{
				wstring parent_path = file_info.FullName.substr(0, file_info.FullName.find_last_of(L'\\'));
				if (out.find(parent_path) == out.end())
					out[parent_path] = vector<FILE_INFO>();
				out[parent_path].push_back(file_info);
			}
		}
	}
}
void ReplaceMFTTime(map<wstring, vector<FILE_INFO>>& out)
{
	map<wstring, MFT_FILE_INFO> data;
	for (map<wstring, vector<FILE_INFO>>::iterator iout= out.begin(); iout != out.end(); iout++)
	{
		QueryMFTPath(iout->first, data);
		for (vector<FILE_INFO>::iterator i = iout->second.begin(); i != iout->second.end(); i++)
			if (data.find(i->Name) != data.end())
			{
				i->CreateTime = data[i->Name].CreateTime;
				i->ModifyTime = data[i->Name].ModifyTime;
				i->AccessTime = data[i->Name].AccessTime;
			}
	}

}

int wmain(size_t argc, WCHAR* argv[])
{
	map<wstring, vector<FILE_INFO>> files;
	QueryPWSDir(LR"(C:\Users\baosa\Desktop)", files);

	ReplaceMFTTime(files);

	for (map<wstring, vector<FILE_INFO>>::iterator i = files.begin(); i != files.end(); i++)
	{
		wprintf(L"%s\n", i->first.c_str());
		for (vector<FILE_INFO>::iterator ii = i->second.begin(); ii != i->second.end(); ii++)
			wprintf(L"\t%s\n", ii->Name.c_str());
	}
}