#pragma warning(disable : 4996)
#pragma comment(lib, "Shlwapi.lib")

#include "mft.h"

#include <Windows.h>
#include "shlwapi.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

#include "NTFS.h"

map<WCHAR, CNTFSVolume*> cache;

//#include "NTFS.h"


void callback(const CIndexEntry* ie, PVOID callback_param)
{
	map<ULONGLONG, MFT_FILE_INFO>* file_map = (map<ULONGLONG, MFT_FILE_INFO>*)callback_param;

	// Hide system metafiles
	if (ie->GetFileReference() < MFT_IDX_USER)
		return;
	
	if ((*file_map).find(ie->GetFileReference()) == (*file_map).end())
		(*file_map)[ie->GetFileReference()] = MFT_FILE_INFO{};
	MFT_FILE_INFO* mft_file = &(*file_map)[ie->GetFileReference()];
		
	//FILETIME ft;
	WCHAR fn[MAX_PATH];
	int fnlen = ie->GetFileName(fn, MAX_PATH);
	if (fnlen > 0)
	{
		ie->GetFileTime(&mft_file->ModifyTime, &mft_file->CreateTime, &mft_file->AccessTime);
		/*SYSTEMTIME st;
		if (FileTimeToSystemTime(&ft, &st))
		{
			printf("%d-%02d-%02d  %02d:%02d\t%s    ", st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, ie->IsDirectory() ? "<DIR>" : "     ");

			if (!ie->IsDirectory())
				printf("%I64u\t", ie->GetFileSize());
			else
				printf("\t");

			printf("<%c%c%c>\t", ie->IsReadOnly() ? 'R' : ' ',
				ie->IsHidden() ? 'H' : ' ', ie->IsSystem() ? 'S' : ' ');
			wprintf(L"%s\n", fn);
		}*/
	}
	if (!ie->IsWin32Name())
		mft_file->DosName = fn;
	else
		mft_file->Name = fn;
}
vector<wstring> split_path(wstring& path)
{
	vector<wstring> ret;
	wstring _path = path;
	size_t pos;
	while ((pos = _path.find_first_of(L'\\')) != string::npos)
	{
		ret.push_back(_path.substr(0, pos));
		_path = _path.substr(pos + 1, _path.length());
	}
	ret.push_back(_path);
	return ret;
}

void QueryMFTPath(std::wstring abs_path, map<wstring, MFT_FILE_INFO>& ret)
{
	ret.clear();

	if (PathIsRelative(abs_path.c_str()))
		return;

	vector<wstring> ppath = split_path(abs_path);
	WCHAR drive_letter = ppath[0][0];
	if (cache.find(drive_letter) == cache.end())
		cache[drive_letter] = new CNTFSVolume(drive_letter);

	CNTFSVolume& volume = *cache[drive_letter];
	if (!volume.IsVolumeOK())
		return;

	// get root directory info
	CFileRecord fr(&volume);
	fr.SetAttrMask(MASK_INDEX_ROOT | MASK_INDEX_ALLOCATION);
	if (!fr.ParseFileRecord(MFT_IDX_ROOT) || !fr.ParseAttrs())
		return;

	// find subdirectory
	CIndexEntry ie;
	for (int i = 1; i < ppath.size(); i++)
		if (!fr.FindSubEntry(ppath[i].c_str(), ie) || !ie.IsDirectory() || !fr.ParseFileRecord(ie.GetFileReference()) || !fr.ParseAttrs())
			return;
	
	map<ULONGLONG, MFT_FILE_INFO> callback_data;
	fr.TraverseSubEntries(callback, &callback_data);

	for (map<ULONGLONG, MFT_FILE_INFO>::iterator i = callback_data.begin(); i != callback_data.end(); i++)
		ret[i->second.Name] = i->second;
	return;
}
