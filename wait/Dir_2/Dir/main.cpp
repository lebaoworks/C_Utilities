#include <Windows.h>


#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
using namespace std;

#include "WinQuery.h"

struct DIR_ENTRY {
    DWORD Error;
    vector<FILE_INFO> Files;
};
void _dir(wstring path, map<wstring, DIR_ENTRY>& entries, BOOL recurse)
{
    queue<wstring> paths;
    paths.push(path);

    wstring p;
    DIR_ENTRY entry;
    while (paths.size() > 0)
    {
        p = paths.front(); paths.pop();
        if (entries.find(p) != entries.end())
            continue;
        
        entry.Error = QueryDirectory(p, entry.Files);
        entries[p] = entry;

        if (recurse)
            for (int i = 0; i < entry.Files.size(); i++)
                if ((entry.Files[i].Attributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (entry.Files[i].Attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0 &&
                    entry.Files[i].Name.compare(L".") != 0 &&
                    entry.Files[i].Name.compare(L"..") != 0)
                    paths.push(entry.Files[i].FullName);
    }
}
void dir(wstring path, PWCHAR* szInfo)
{
    if (szInfo == NULL)
        return;

    map<wstring, DIR_ENTRY> entries;
    _dir(LR"(D:\Desktop)", entries, TRUE);

#define HUG_SIZE 65536
    *szInfo = (PWCHAR) calloc(HUG_SIZE+1, sizeof(WCHAR));
    if (*szInfo == NULL)
        return;

    for (map<wstring, DIR_ENTRY>::iterator entry = entries.begin(); entry != entries.end(); entry++)
    {
        wcout << entry->first << endl;
        wcscat_s(*szInfo, HUG_SIZE, entry->first.c_str());
        if (entry->second.Error != 0)
        {
            LPWSTR messageBuffer = NULL;
            if (0 != FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, entry->second.Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL))
            {
                wcscat_s(*szInfo, HUG_SIZE, messageBuffer);
                LocalFree(messageBuffer);
            }
            else
                wcscat_s(*szInfo, HUG_SIZE, (wstring(L"Got Error ") + to_wstring(entry->second.Error)).c_str());
        }
        else
        {

            for (int i = 0; i < entry->second.Files.size(); i++)
            {
                //wcout << "\tName:" << entry->second.Files[i].Name << endl;
                //wcout << "\tOwner SID: " << entry->second.Files[i].Owner.SID << endl;
                //wcout << "\tOwner Name: " << entry->second.Files[i].Owner.DomainName + L"\\" + entry->second.Files[i].Owner.Name << endl;
            }
        }
    }
}

int main()
{
    PWCHAR szInfo;
    dir(LR"(D:\Desktop)", &szInfo);
    fwrite(szInfo, 2, wcslen(szInfo), stdout);
}