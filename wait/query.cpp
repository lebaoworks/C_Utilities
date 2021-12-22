// processquery.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Wtsapi32.lib")

#define _WIN32_DCOM
#define UNICODE

#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <Wtsapi32.h>

#include <cstring>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>


using namespace std;

struct ProcessInfo {
    wstring imagePath;
    wstring imageName;
    DWORD pid;
    DWORD ppid;
    wstring sessionName;
    DWORD sessionID;
    ULONGLONG memUsage;
    wstring status;
    wstring domainName;
    wstring userName;
    ULONGLONG cpuTime;
    wstring windowTitle;
};
ProcessInfo NewProcessInfo()
{
    ProcessInfo x;
    x.memUsage = -1;
    x.cpuTime = -1;
    return x;
}

const wstring WHITESPACE = L" \n\r\t\f\v";
wstring ltrim(wstring& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::wstring::npos) ? L"" : s.substr(start);
}
wstring rtrim(wstring& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? L"" : s.substr(0, end + 1);
}
wstring trim(wstring& s) {
    wstring x = ltrim(s);
    return rtrim(x);
}
void RunCmdAndGetText(LPWSTR, vector<wstring>&);


// Query from WMI
//      Get:    ImageName (.exe)
//              ImagePath
//              pid
//              ppid
//              sessionID
// Reference:   https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/WmiSdk/example--getting-wmi-data-from-the-local-computer.md
//              https://stackoverflow.com/a/42848834
//              https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-process
// Author: lebaoworks
DWORD QueryWMI(map<DWORD, ProcessInfo>& data)
{
    vector<ProcessInfo> ret;
    HRESULT hres;
    SYSTEM_INFO sSysInfo; GetSystemInfo(&sSysInfo);
    DWORD dwPageSize = sSysInfo.dwPageSize;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return 1;   // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );


    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return 1;   // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return 1;   // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices* pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
            << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return 1;   // Program has failed.
    }

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;   // Program has failed.
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Process"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        cout << "Query for operating system name failed."
            << " Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;   // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;     VariantInit(&vtProp);
        VARIANT vtDomain;   VariantInit(&vtDomain);
        VARIANT vtUsername; VariantInit(&vtUsername);

        ProcessInfo x = NewProcessInfo();
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        if (vtProp.bstrVal != NULL)
            x.imageName = vtProp.bstrVal;

        hr = pclsObj->Get(L"ExecutablePath", 0, &vtProp, 0, 0);
        if (vtProp.bstrVal != NULL)
            x.imagePath = vtProp.bstrVal;

        hr = pclsObj->Get(L"ProcessId", 0, &vtProp, 0, 0);
        x.pid = vtProp.ulVal;

        hr = pclsObj->Get(L"ParentProcessId", 0, &vtProp, 0, 0);
        x.ppid = vtProp.ulVal;

        hr = pclsObj->Get(L"SessionId", 0, &vtProp, 0, 0);
        x.sessionID = vtProp.ulVal;

        data[x.pid] = x;
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return 0;
}

// Query from PowerShell
//      Get:    pid
//              memUsage
//              cpuTime
//              status
//              domainName
//              userName
//              windowTitle
// Author: lebaoworks
void QueryPWS(map<DWORD, ProcessInfo>& data)
{
    TCHAR szCmdline[] = L"powershell.exe -c"
        " \""
        "Get-Process -IncludeUserName |"
        "Select-Object -Property"
        "   Id,"
        "   WorkingSet,"
        "   @{Label = 'CPU(s)'; Expression = {if ($_.CPU) {[Math]::Floor($_.CPU)} else {'N/A'}}},"
        "   @{Label = 'Status'; Expression = {"
        "       $run='Suspended';"
        "       foreach ($thread in $_.Threads) {   "
        "           if ($thread.ThreadState -eq 'Wait') {"
        "               if ($($thread.WaitReason.ToString()) -ne 'Suspended') {"
        "                   $run = 'Running'"
        "               }"
        "           }"
        "       };"
        "       $run"
        "   }},"
        "   Username,"
        "   @{Label='separator'; Expression={';'}},"
        "    mainWindowTitle |"
        "Format-Table -Wrap"
        "\"";
    vector<wstring> output;
    RunCmdAndGetText(szCmdline, output);
    for (int i = 3; i < output.size() - 3; i++)
    {
        ProcessInfo x = NewProcessInfo();
        wstring e;
        wstringstream s = wstringstream(output[i]);
        s >> e; x.pid = _wtoi(e.c_str());
        s >> e; x.memUsage = _wtoi(e.c_str());
        s >> e;
        if (e.compare(L"N/A") == 0)
            x.cpuTime = -1;
        else
            x.cpuTime = _wtoi(e.c_str());
        s >> e; x.status = e;

        // get username, may be empty so need separator
        getline(s, e, L';');
        e = trim(e);
        if (e.compare(L"") != 0)
        {
            x.domainName = e.substr(0, e.find_first_of(L"\\", 0));
            x.userName = e.substr(x.domainName.length() + 1, e.length());
        }
        // get windows title, the rest of line
        getline(s, e);
        e = trim(e);
        x.windowTitle = e;

        data[x.pid] = x;
    }
}
vector<ProcessInfo> QueryProcess()
{
    map<DWORD, ProcessInfo> info_wmi;
    QueryWMI(info_wmi);
    map<DWORD, ProcessInfo> info_pws;
    QueryPWS(info_pws);
    map<DWORD, wstring> session_cache;

    vector<ProcessInfo> ret;
    for (map<DWORD, ProcessInfo>::iterator it = info_wmi.begin(); it != info_wmi.end(); ++it)
    {
        map<DWORD, ProcessInfo>::iterator pws_ite = info_pws.find(it->first);
        if (pws_ite != info_pws.end())
        {
            it->second.memUsage = pws_ite->second.memUsage;
            it->second.cpuTime = pws_ite->second.cpuTime;
            it->second.status = pws_ite->second.status;
            it->second.domainName = pws_ite->second.domainName;
            it->second.userName = pws_ite->second.userName;
            it->second.windowTitle = pws_ite->second.windowTitle;

            if (session_cache.find(it->second.sessionID) != session_cache.end())
                it->second.sessionName = session_cache[it->second.sessionID];
            else
            {
                WTSINFO* pWTSInfo;
                DWORD retLen;
                if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, it->second.sessionID, WTSSessionInfo, (LPWSTR*)&pWTSInfo, &retLen))
                {
                    it->second.sessionName = pWTSInfo->WinStationName;
                    WTSFreeMemory(pWTSInfo);
                }
                session_cache[it->second.sessionID] = it->second.sessionName;
            }
            ret.push_back(it->second);
        }
    }
    return ret;
}

int main(int argc, char** argv)
{
    CHAR* szInfo = (CHAR*)calloc(1, 65535);

    map<string, size_t> widths = {
        {"Name", 24},
        {"PID", 8},
        {"PPID", 8},
        {"SessionName", 16},
        {"SessionID", 12},
        {"MemUsage", 12},
        {"Status", 12},
        {"User", 32},
        {"Time", 12},
        {"Title", 20},
    };
    WCHAR szLineFormatR[] = L"%%-%ds %%%ds %%%ds %%-%ds %%%ds %%%ds %%-%ds %%-%ds %%%ds %%-%ds\n";
    WCHAR szLineFormat[256];
    swprintf_s(szLineFormat, 256, szLineFormatR,
        widths["Name"],
        widths["PID"],
        widths["PPID"],
        widths["SessionName"],
        widths["SessionID"],
        widths["MemUsage"],
        widths["Status"],
        widths["User"],
        widths["Time"],
        widths["Title"]
    );

    vector<ProcessInfo> infos = QueryProcess();
    wprintf(szLineFormat,
        L"Image Name",
        L"PID",
        L"PPID",
        L"Session Name",
        L"Session##",
        L"Mem Usage",
        L"Status",
        L"User",
        L"Time",
        L"Title"
    );
    for (int i = 0; i < widths["Name"]; i++) { printf("="); } printf(" ");
    for (int i = 0; i < widths["PID"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["PPID"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["SessionName"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["SessionID"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["MemUsage"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["Status"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["User"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["Time"]; i++) { printf("="); }  printf(" ");
    for (int i = 0; i < widths["Title"]; i++) { printf("="); }  printf(" ");
    printf("\n");

    for (int i = 0; i < infos.size(); i++)
    {
        if (infos[i].imageName.length() > widths["Name"])
            infos[i].imageName.resize(widths["Name"]);
        wchar_t cpuTimeStr[20];
        if (infos[i].cpuTime == -1)
            swprintf_s(cpuTimeStr, 20, L"N/A");
        else
            swprintf_s(cpuTimeStr, 20, L"%d:%02d:%02d", infos[i].cpuTime / 3600, (infos[i].cpuTime % 3600) / 60, infos[i].cpuTime % 60);

        wprintf(szLineFormat,
            infos[i].imageName.c_str(),
            to_wstring(infos[i].pid).c_str(),
            to_wstring(infos[i].ppid).c_str(),
            infos[i].sessionName.c_str(),
            to_wstring(infos[i].sessionID).c_str(),
            (to_wstring(infos[i].memUsage / 1024) + L" K").c_str(),
            infos[i].status.c_str(),
            (infos[i].userName.compare(L"") == 0) ? L"N/A" : (infos[i].domainName + L"\\" + infos[i].userName).c_str(),
            cpuTimeStr,
            infos[i].windowTitle.c_str()
        );
    }
    return 0;
}

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

