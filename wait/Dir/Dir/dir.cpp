//
// DIR <absolute_path>
//		/A:[-]attributes		Displays files with specified attributes.
//			attributes	D  Directories		R  Read-only files
//						H  Hidden files		A  Files ready for archiving
//						S  System files		
//						L  Reparse Points	
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

#include <Windows.h>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <regex>

using namespace std;

#include "WinQuery.h"
#include "mft.h"
#include "cpp_string.h"

#define SORT_NAME 0x00000000
#define SORT_SIZE 0x00000001
#define SORT_EXT  0x00000002
#define SORT_DATE 0x00000003
#define SORT_DIR  0x00000004

#define TIME_CREATE     0x00000000
#define TIME_MODIFY     0x00000001
#define TIME_ACCESS     0x00000002
#define TIME_MFT_CREATE 0x00000003
#define TIME_MFT_MODIFY 0x00000004
#define TIME_MFT_ACCESS 0x00000005

map<wstring, wstring> WidthFix = {
    {L"Time" ,      L"%18.18s"},
    {L"Attributes", L"%7.7s"},
    {L"Size",       L"%15.15s"},
    {L"DosName",    L"%-15.15s"},
    {L"Owner",      L"%-30.30s"}
};


struct DIR_ENTRY {
    DWORD Error;
    vector<FILE_INFO> Files;
};
void _dir(wstring path, map<wstring, DIR_ENTRY>& entries, BOOL recurse)
{
    DIR_ENTRY entry;
    if (entries.find(path) != entries.end())
        return;
        
    entry.Error = QueryDirectory(path, entry.Files);
    entries[path] = entry;

    if (recurse)
        for (int i = 0; i < entry.Files.size(); i++)
            if ((entry.Files[i].Attributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (entry.Files[i].Attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0 &&
                entry.Files[i].Name.compare(L".") != 0 &&
                entry.Files[i].Name.compare(L"..") != 0)
                _dir(entry.Files[i].FullName, entries, recurse);
}

bool parse_flags(vector<wstring> iflags, map<string, DWORD>& flags)
{
    flags.clear();

    // lower case
    for (int i = 0; i < iflags.size(); i++)
    {
        for (int j = 0; j < iflags[i].length(); j++)
            iflags[i][j] = towlower(iflags[i][j]);
        if (!regex_match(iflags[i], wregex(LR"(/[alotqrsx].*)")))
            return false;
    }

    // parse flags
    // /a
    flags["attr_inc"] = 0;
    flags["attr_exc"] = 0;
    // /l
    flags["lower_name"] = 0;
    // /o
    flags["sort_reverse"] = 0;
    flags["sort_field"] = 0;
    // /t
    flags["time_field"] = 0;
    flags["time_reverse"] = 0;
    // /q
    flags["show_owner"] = 0;
    // /r
    flags["show_alt_stream"] = 0;
    // /s
    flags["recurse"] = 0;
    // /x
    flags["show_dos_name"] = 0;
    wsmatch match;
    for (int i = 0; i < iflags.size(); i++)
    {
        if (iflags[i].length() < 2)
            return FALSE;
        wstring flag_name = iflags[i].substr(0, 2);
        wstring flag_content = (iflags[i].length() > 3) ? iflags[i].substr(3, iflags[i].length()) : L"";
        if (flag_name.compare(0, 2, L"/a") == 0)
        {
            wregex flag_attr_exp(LR"(-?[drhslo])");
            while (regex_search(flag_content, match, flag_attr_exp))
            {
                DWORD* x = &flags["attr_inc"];
                if (match.str().front() == L'-')
                    x = &flags["attr_exc"];
                if (match.str().back() == L'd')
                    *x |= FILE_ATTRIBUTE_DIRECTORY;
                else if (match.str().back() == L'r')
                    *x |= FILE_ATTRIBUTE_READONLY;
                else if (match.str().back() == L'h')
                    *x |= FILE_ATTRIBUTE_HIDDEN;
                else if (match.str().back() == L'a')
                    *x |= FILE_ATTRIBUTE_ARCHIVE;
                else if (match.str().back() == L's')
                    *x |= FILE_ATTRIBUTE_SYSTEM;
                else if (match.str().back() == L'l')
                    *x |= FILE_ATTRIBUTE_REPARSE_POINT;
                else
                    return FALSE;
                flag_content = match.suffix();
            }
        }
        else if (flag_name.compare(0, 2, L"/l") == 0)
        {
            flags["lower_name"] = 1;
        }
        else if (flag_name.compare(0, 2, L"/o") == 0)
        {
            if (!regex_match(flag_content, wregex(LR"(-?[nsedg])")))
                return false;
            if (flag_content.front() == L'-')
                flags["sort_reverse"] = 1;
            if (flag_content.back() == L'n')
                flags["sort_field"] = SORT_NAME;
            else if (flag_content.back() == L's')
                flags["sort_field"] = SORT_SIZE;
            else if (flag_content.back() == L'e')
                flags["sort_field"] = SORT_EXT;
            else if (flag_content.back() == L'd')
                flags["sort_field"] = SORT_DATE;
            else if (flag_content.back() == L'g')
                flags["sort_field"] = SORT_DIR;
            else
                return false;
        }
        else if (flag_name.compare(0, 2, L"/t") == 0)
        {
            if (!regex_match(flag_content, wregex(LR"(-?[cmaw])")))
                return false;
            if (flag_content.front() == L'-')
                flags["time_reverse"] = 1;
            if (flag_content.back() == L'c')
                flags["time_field"] = TIME_CREATE;
            else if (flag_content.back() == L'm')
                flags["time_field"] = TIME_MFT_CREATE;
            else if (flag_content.back() == L'a')
                flags["time_field"] = TIME_ACCESS;
            else if (flag_content.back() == L'w')
                flags["time_field"] = TIME_MFT_MODIFY;
            else
                return false;
        }
        else if (flag_name.compare(0, 2, L"/q") == 0)
        {
            flags["show_owner"] = 1;
        }
        else if (flag_name.compare(0, 2, L"/r") == 0)
        {
            flags["show_alt_stream"] = 1;
        }
        else if (flag_name.compare(0, 2, L"/s") == 0)
        {
            flags["recurse"] = 1;
        }
        else if (flag_name.compare(0, 2, L"/x") == 0)
        {
            flags["show_dos_name"] = 1;
        }
    }

    if (flags["attr_inc"] == 0)
        flags["attr_inc"] = ~flags["attr_inc"];
    
    return true;
}

void patch_mft(map<wstring, DIR_ENTRY>& entries)
{
    map<wstring, MFT_FILE_INFO> data;
    for (map<wstring, DIR_ENTRY>::iterator entry = entries.begin(); entry != entries.end(); entry++)
    {
        if (entry->second.Error != 0)
            continue;
        QueryMFTPath(entry->first, data);
        for (vector<FILE_INFO>::iterator file = entry->second.Files.begin(); file != entry->second.Files.end(); file++)
        {
            map<wstring, MFT_FILE_INFO>::iterator mft_info = data.find(file->Name);
            if (mft_info != data.end())
            {
                file->MFTAccessTime = mft_info->second.AccessTime;
                file->MFTCreateTime = mft_info->second.CreateTime;
                file->MFTModifyTime = mft_info->second.ModifyTime;
            }
        }
    }
}

#include <iostream>
void dir(wstring path, vector<wstring> input_flags, wstringstream& out)
{
    // clear output
    out.str(L"");

    // parse flags
    map<string, DWORD> flags;
    if (!parse_flags(input_flags, flags))
    {
        out << "Parameters invalid!";
        return;
    }

    // query dir
    map<wstring, DIR_ENTRY> entries;
    _dir(LR"(D:\Desktop)", entries, flags["recurse"]);
    
    // patch mft
    if (flags["time_field"] == TIME_MFT_ACCESS ||
        flags["time_field"] == TIME_MFT_CREATE ||
        flags["time_field"] == TIME_MFT_MODIFY)
        patch_mft(entries);

    // write output
    wstringstream line;
    for (map<wstring, DIR_ENTRY>::iterator entry = entries.begin(); entry != entries.end(); entry++)
    {
        out << "\n\t" << entry->first << L"\n\n";
        if (entry->second.Error != 0)
        {
            LPWSTR messageBuffer = NULL;
            if (0 != FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, entry->second.Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL))
            {
                out << messageBuffer << L'\n';
                LocalFree(messageBuffer);
            }
            else
                out << wstring(L"Got Error ") + to_wstring(entry->second.Error) << L'\n';
            continue;
        }
        for (vector<FILE_INFO>::iterator file = entry->second.Files.begin(); file!=entry->second.Files.end(); file++)
        {
            // apply filter
            if ((file->Attributes & flags["attr_inc"]) == 0 || (file->Attributes & flags["attr_exc"]) != 0)
                continue;

            // parse time
            SYSTEMTIME st = {};
            if (flags["time_field"] == TIME_ACCESS)
                FileTimeToSystemTime(&file->AccessTime, &st);
            else if (flags["time_field"] == TIME_CREATE)
                FileTimeToSystemTime(&file->CreateTime, &st);
            else if (flags["time_field"] == TIME_MODIFY)
                FileTimeToSystemTime(&file->ModifyTime, &st);
            else if (flags["time_field"] == TIME_MFT_ACCESS)
                FileTimeToSystemTime(&file->MFTAccessTime, &st);
            else if (flags["time_field"] == TIME_MFT_CREATE)
                FileTimeToSystemTime(&file->MFTCreateTime, &st);
            else if (flags["time_field"] == TIME_MFT_MODIFY)
                FileTimeToSystemTime(&file->MFTModifyTime, &st);
            wstring time = string_format(L"%4d/%02d/%02d  %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
            
            // parse attr
            wstring attr = string_format(L"%c%c%c%c%c%c",
                (file->Attributes & FILE_ATTRIBUTE_REPARSE_POINT) ? 'l' : '-',
                (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '-',
                (file->Attributes & FILE_ATTRIBUTE_ARCHIVE) ? 'a' : '-',
                (file->Attributes & FILE_ATTRIBUTE_READONLY) ? 'r' : '-',
                (file->Attributes & FILE_ATTRIBUTE_HIDDEN) ? 'h' : '-',
                (file->Attributes & FILE_ATTRIBUTE_SYSTEM) ? 's' : '-');

            // parse size
            wstring size = (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) ? L"": to_wstring(file->Size.QuadPart);
            // parse owner name
            wstring owner = L"<Unknown>";
            if (file->Owner.Name.length() != 0)
                owner = file->Owner.DomainName + L"\\" + file->Owner.Name;
            else if (file->Owner.SID.length() != 0)
                owner = L"<Foreign Owner>";

            // parse name
            wstring name = file->Name;
            if (flags["lower_name"])
                for (int i = 0; i < name.length(); i++)
                    name[i] = towlower(name[i]);
            
            line.str(L"");
            line << string_format(WidthFix[L"Time"], time.c_str());
            line << " " << string_format(WidthFix[L"Attributes"], attr.c_str());
            line << " " << string_format(WidthFix[L"Size"], size.c_str());
            if (flags["show_dos_name"])
                line << " " << string_format(WidthFix[L"DosName"], file->DosName.c_str());
            if (flags["show_owner"])
                line << " " << string_format(WidthFix[L"Owner"], owner.c_str());

            line << " " << name << '\n';
            if (flags["show_alt_stream"])
                for (int i = 0; i < file->Streams.size(); i++)
                {
                    line << string_format(WidthFix[L"Time"], L"");
                    line << " " << string_format(WidthFix[L"Attributes"], L"");
                    line << " " << string_format(WidthFix[L"Size"], to_wstring(file->Streams[i].Size.QuadPart).c_str());
                    if (flags["show_dos_name"])
                        line << " " << string_format(WidthFix[L"DosName"], L"");
                    if (flags["show_owner"])
                        line << " " << string_format(WidthFix[L"Owner"], L"");
                    line << " " << name + file->Streams[i].Name << '\n';
                }
            out << line.str();
        }
    }
}

