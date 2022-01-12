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

BOOL AssertParameters(vector<wstring>& params)
{
	return TRUE;
}
void ParseDirCommand(vector<wstring> lines, map<wstring, vector<wstring>>& out)
{
	wstring dir_path = L"";
	for (int i = 0; i < lines.size(); i++)
		if (lines[i].compare(0, 13, L" Directory of") == 0)
		{
			dir_path = lines[i].substr(13, lines[i].length());
			trim(dir_path);
			out[dir_path] = vector<wstring>();
			wprintf(L"New path: %s\n", dir_path.c_str());
		}
		else
			if (dir_path.length() > 0)
			{
				trim(lines[i]);
				if (lines[i].length() > 0)
				{
					out[dir_path].push_back(lines[i]);
					wprintf(L"\t%s\n", lines[i].c_str());
				}
			}
}
void ReplaceMFTTime(map<wstring, vector<wstring>>& out)
{
	map<wstring, MFT_FILE_INFO> data;
	wregex dir_file_exp = wregex(LR"((\d{2}\/\d{2}\/\d{4}\s+\d{2}:\d{2}\s+[AP]M)\s+([\d,]+)\s+(.+))");
	for (map<wstring, vector<wstring>>::iterator imap = out.begin(); imap != out.end(); imap++)
	{
		QueryMFTPath(imap->first, data);
		for (vector<wstring>::iterator i=imap->second.begin(); i!=imap->second.end(); i++)
			wprintf(L"%s\n", i->c_str());
	}
	
}
int wmain(size_t argc, WCHAR* argv[])
{
	vector<wstring> lines;
	RunCmdAndGetText(LR"(cmd.exe /c "dir /X C:\Users\baosa\Desktop")", lines);

	map<wstring, vector<wstring>> out;
	ParseDirCommand(lines, out);

	ReplaceMFTTime(out);
}