// 
// /A:
// /O: 
// /T
// /L
// /Q
// /R
// /S
// /X
#pragma warning(disable : 4996)
#define UNICODE

#include "NTFS.h"
#include <stdio.h>

void callback(const CIndexEntry* ie)
{
	// Hide system metafiles
	if (ie->GetFileReference() < MFT_IDX_USER)
		return;

	// Ignore DOS alias file names
	if (!ie->IsWin32Name())
		return;

	FILETIME ft;
	WCHAR fn[MAX_PATH];
	int fnlen = ie->GetFileName(fn, MAX_PATH);
	if (fnlen > 0)
	{
		ie->GetFileTime(&ft);
		SYSTEMTIME st;
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
		}

	}
}
#include <vector>
#include <string>
using namespace std;
vector<wstring> split_path(WCHAR* path)
{
	vector<wstring> ret;
	wstring _path = path;
	size_t pos;
	while ((pos = _path.find_first_of(L'\\')) != string::npos)
	{
		ret.push_back(_path.substr(0, pos));
		_path = _path.substr(pos + 1, _path.length());
	}
	return ret;
}

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
	{
		return -1;
	}
	
	WCHAR* path = argv[1];

	vector<wstring> ppath = split_path(path);
	WCHAR volname;
	volname = ppath[0][0];
	if (!volname)
	{
		return -1;
	}

	CNTFSVolume volume(volname);
	if (!volume.IsVolumeOK())
	{
		printf("Cannot get NTFS BPB from boot sector of volume %c\n", volname);
		return -1;
	}

	// get root directory info

	CFileRecord fr(&volume);

	// we only need INDEX_ROOT and INDEX_ALLOCATION
	// don't waste time and ram to parse unwanted attributes
	fr.SetAttrMask(MASK_INDEX_ROOT | MASK_INDEX_ALLOCATION);

	if (!fr.ParseFileRecord(MFT_IDX_ROOT))
	{
		printf("Cannot read root directory of volume %c\n", volname);
		return -1;
	}

	if (!fr.ParseAttrs())
	{
		printf("Cannot parse attributes\n");
		return -1;
	}

	// find subdirectory

	WCHAR pathname[MAX_PATH];
	int pathlen;

	//while (1)
	//{
	//	pathlen = getpathname(&path, pathname);
	//	if (pathlen < 0)	// parameter syntax error
	//	{
	//		usage();
	//		return -1;
	//	}
	//	if (pathlen == 0)
	//		break;	// no subdirectories

	//	CIndexEntry ie;
	//	if (fr.FindSubEntry(pathname, ie))
	//	{
	//		if (ie.IsDirectory())
	//		{
	//			if (!fr.ParseFileRecord(ie.GetFileReference()))
	//			{
	//				printf("Cannot read directory %s\n", pathname);
	//				return -1;
	//			}
	//			if (!fr.ParseAttrs())
	//			{
	//				printf("Cannot parse attributes\n");
	//				return -1;
	//			}
	//		}
	//		else
	//		{
	//			printf("%s is not a directory\n", pathname);
	//			return -1;
	//		}
	//	}
	//	else
	//	{
	//		printf("Cannot find directory %s\n", pathname);
	//		return -1;
	//	}
	//}

	//// list it !

	//fr.TraverseSubEntries(printfile);

	//printf("Files: %d, Directories: %d\n", totalfiles, totaldirs);

	return 0;
}
