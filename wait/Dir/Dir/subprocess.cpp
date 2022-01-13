#include "subprocess.h"


#include <Windows.h>

#include <vector>
#include <string>
#include <sstream>

using namespace std;

void RunCmdAndGetText(wstring cmd_line, vector<wstring>& output)
{
    output.clear();

    // Setup Pipe's attr
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    // Create a pipe for the child process's STDOUT. 
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        return;
    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(g_hChildStd_OUT_Rd);
        CloseHandle(g_hChildStd_OUT_Wr);
        return;
    }

    PROCESS_INFORMATION piProcInfo;     ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    STARTUPINFO siStartInfo;            ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    // Specifies the STDIN and STDOUT handles for redirection.
    siStartInfo.cb = sizeof(STARTUPINFO);
    //siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    //siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    siStartInfo.wShowWindow = SW_HIDE;

    // Set Console Ouput Format
    AllocConsole();
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // Setup cmd buffer
    WCHAR* szCmdLine = (WCHAR*)calloc(cmd_line.length() + 1, sizeof(WCHAR));
    if (szCmdLine == NULL)
        return;
    memcpy(szCmdLine, cmd_line.c_str(), sizeof(WCHAR) * cmd_line.length());

    // Create the child process. 
    BOOL bSuccess = CreateProcess(NULL,
        szCmdLine,          // command line 
        NULL,               // process security attributes 
        NULL,               // primary thread security attributes 
        TRUE,               // handles are inherited 
        0,                  // creation flags 
        NULL,               // use parent's environment 
        NULL,               // use parent's current directory 
        &siStartInfo,       // STARTUPINFO pointer 
        &piProcInfo);       // receives PROCESS_INFORMATION

    // Free cmd buffer
    free(szCmdLine);

     // If an error occurs, exit the application. 
    if (!bSuccess)
        return;
    else
    {
        // Communicating code here...

        // Close handles to the child process and its primary thread.
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        CloseHandle(g_hChildStd_OUT_Wr);
    }

    // Read output from the child process's pipe for STDOUT
    // and write to the parent process's pipe for STDOUT. 
    // Stop when there is no more data. 
    stringstream s;
    DWORD dwRead;
    CHAR chBuf[4096];
    for (;;)
    {
        if (!ReadFile(g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL) || dwRead == 0)
            break;
        s.write(chBuf, dwRead);
    }
    CloseHandle(g_hChildStd_OUT_Rd);

    // Convert output to wchar string
    DWORD code_page = GetConsoleCP();
    int needed = MultiByteToWideChar(code_page, 0, s.str().c_str(), s.str().length(), NULL, 0);
    if (needed == 0)
        return;
    WCHAR* buffer = (WCHAR*)calloc(needed, sizeof(WCHAR));
    MultiByteToWideChar(code_page, 0, s.str().c_str(), s.str().length(), buffer, needed);
    
    wstringstream ws;
    ws.write(buffer, needed);
    free(buffer);
    
    // Split to lines
    wstring line;
    while (!ws.eof())
    {
        getline(ws, line);
        output.push_back(line);
    }
}

