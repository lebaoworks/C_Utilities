#include <Windows.h>

#include <iostream>
#include <vector>

#include "DllUtils.h"
#include "kernel_ntdll.h"

using namespace std;

int main2(int argc, char* argv[])
{
	vector<MODULE_INFO> list;
	if (argc != 2)
		return 1;

	DWORD iStatus = GetModules(atoi(argv[1]), list);
	if (iStatus != 0)
	{
		cout << "Got: " << iStatus << endl;
		return 1;
	}

	for (int i = 0; i < list.size(); i++)
	{
		wcout << list[i].FullDllName << " " << list[i].DllBase << " " << list[i].SizeOfImage << endl;
	}
	return 0;
}

#include <psapi.h>
int main(int argc, char* argv[])
{
	vector<MODULE_INFO> list;
	if (argc != 2)
		return 1;
	
	DWORD pid = atoi(argv[1]);
	/*GetModules(atoi(argv[1]), list);

	for (int i = 0; i < list.size(); i++)
	{
		wcout << list[i].BaseDllName << "\t" << list[i].DllBase << "\t" << list[i].LoadCount << endl;
	}*/


    char szBuf[MAX_PATH * 100] = { 0 };
    PBYTE pb = NULL;
    MEMORY_BASIC_INFORMATION mbi;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL)
    {
        cout << "OpenProcess fail: " << GetLastError() << endl;
        return 1;
    }

    while (VirtualQueryEx(hProcess, pb, &mbi, sizeof(mbi)) == sizeof(mbi)) {

        int nLen;
        WCHAR szModName[MAX_PATH];
        WCHAR szFileName[MAX_PATH];

        if (mbi.State == MEM_FREE)
            mbi.AllocationBase = mbi.BaseAddress;

        if ((mbi.AllocationBase != mbi.BaseAddress) ||
            (mbi.AllocationBase == NULL)) {
            // Do not add the module name to the list
            // if any of the following is true:
            // 1. If this region contains this DLL
            // 2. If this block is NOT the beginning of a region
            // 3. If the address is NULL
            nLen = 0;
        }
        else {
            printf("-> %p\n", mbi.AllocationBase);
            nLen = GetModuleFileNameEx(hProcess, (HMODULE) mbi.AllocationBase, szModName, _countof(szModName));
            if (nLen == 0)
            {
                if (GetMappedFileName(hProcess, mbi.AllocationBase, szModName, MAX_PATH) != 0)
                    wcout << "-->" << szModName << endl;
                else
                    cout << "--> " << GetLastError() << endl;
            }
            else
                wcout << "-> [*] " << szModName << endl;
        }

        pb += mbi.RegionSize;
    }

}