#include <Windows.h>

#include <vector>
#include <string>
#include <sstream>

using namespace std;

void RunCmdAndGetText(LPWSTR szCmdline, vector<wstring>& output)
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

    // Create the child process. 
    BOOL bSuccess = CreateProcess(NULL,
        szCmdline,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

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
    wstringstream s;
    DWORD dwRead, dwWritten;
    CHAR chBuf[4096 + 1];
    bSuccess = FALSE;
    MultiByteToWideChar(GetConsoleOutputCP()
        HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;
        chBuf[dwRead] = 0;
        s << chBuf;
    }
    CloseHandle(g_hChildStd_OUT_Rd);

    wstring line;
    while (!s.eof())
    {
        getline(s, line);
        output.push_back(line);
    }
}

