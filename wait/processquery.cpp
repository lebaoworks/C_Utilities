// processquery.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma comment(lib, "wbemuuid.lib")

#define _WIN32_DCOM
#define UNICODE

#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>


using namespace std;

struct ProcessInfo {
    wstring imageName;
    DWORD pid = 0;
    DWORD ppid = 0;
    wstring sessionName;
    DWORD sessionID = 0;
    ULONGLONG memUsage = 0;
    wstring status;
    wstring userName;
    ULONGLONG cpuTime = 0;
    wstring title;
};

map<string, size_t> widths = {
    {"Name", 24},
    {"PID", 8},
    {"PPID", 8},
    {"SessionName", 16},
    {"SessionID", 12},
    {"Mem", 12},
    {"Status", 21},
    {"User", 20},
    {"Time", 8},
    {"Title", 20},
};

// Query from WMI
// Reference:   https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/WmiSdk/example--getting-wmi-data-from-the-local-computer.md
//              https://stackoverflow.com/a/42848834
//              https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-process
// Author: lebaoworks
vector<ProcessInfo> QueryWMI()
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
        return vector<ProcessInfo>();   // Program has failed.
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
        return vector<ProcessInfo>();   // Program has failed.
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
        return vector<ProcessInfo>();   // Program has failed.
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
        return vector<ProcessInfo>();   // Program has failed.
    }

    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


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
        return vector<ProcessInfo>();   // Program has failed.
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
        return vector<ProcessInfo>();   // Program has failed.
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

        ProcessInfo x;
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        x.imageName = vtProp.bstrVal;

        hr = pclsObj->Get(L"ProcessId", 0, &vtProp, 0, 0);
        x.pid = vtProp.ulVal;

        hr = pclsObj->Get(L"ParentProcessId", 0, &vtProp, 0, 0);
        x.ppid = vtProp.ulVal;

        hr = pclsObj->Get(L"SessionId", 0, &vtProp, 0, 0);
        x.sessionID = vtProp.ulVal;

        x.sessionName = L"N/A";

        hr = pclsObj->Get(L"PageFileUsage", 0, &vtProp, 0, 0);
        x.memUsage = vtProp.ulVal;
        
        hr = pclsObj->Get(L"UserModeTime", 0, &vtProp, 0, 0);
        x.cpuTime = vtProp.ullVal;
        // Get Owner
        x.userName = L"N/A";

        ret.push_back(x);

        VariantClear(&vtProp);
        pclsObj->Release();
    }

    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return ret;   // Program successfully completed.
}
void FillInfo(vector<ProcessInfo>& infos)
{

}
int main(int argc, char** argv)
{
    vector<ProcessInfo> infos = QueryWMI();
    FillInfo(infos);

    {
        wcout << std::left << std::setw(widths["Name"]) << "Process Name";
        wcout << " ";
        wcout << std::right << std::setw(widths["PID"]) << "PID";
        wcout << " ";
        wcout << std::right << std::setw(widths["PPID"]) << "PPID";
        wcout << " ";
        wcout << std::left << std::setw(widths["SessionName"]) << "Session Name";
        wcout << " ";
        wcout << std::right << std::setw(widths["SessionID"]) << "Session ID";
        wcout << " ";
        wcout << std::right << std::setw(widths["Mem"]) << "Mem Usage";
        wcout << " ";
        wcout << std::left << std::setw(widths["Status"]) << "Status";
        wcout << " ";
        wcout << std::left << std::setw(widths["User"]) << "User Name";
        wcout << " ";
        wcout << std::right << std::setw(widths["Time"]) << "CPU Time (ms)";
        wcout << " ";
        wcout << std::left << std::setw(widths["Title"]) << "Windows Title";
        wcout << endl << endl;
    }
    for (int i = 0; i < infos.size(); i++)
    {
        if (infos[i].imageName.size() > widths["Name"])
            infos[i].imageName.resize(widths["Name"]);
        wcout << std::left << std::setw(widths["Name"]) << infos[i].imageName;
        wcout << " ";
        wcout << std::right << std::setw(widths["PID"]) << infos[i].pid;
        wcout << " ";
        wcout << std::right << std::setw(widths["PPID"]) << infos[i].ppid;
        wcout << " ";
        wcout << std::left << std::setw(widths["SessionName"]) << infos[i].sessionName;
        wcout << " ";
        wcout << std::right << std::setw(widths["SessionID"]) << infos[i].sessionID;
        wcout << " ";
        wcout << std::right << std::setw(widths["Mem"]) << to_wstring(infos[i].memUsage) + L" K";
        wcout << " ";
        wcout << std::left << std::setw(widths["Status"]) << infos[i].status;
        wcout << " ";
        wcout << std::left << std::setw(widths["User"]) << infos[i].userName;
        wcout << " ";
        wcout << std::right << std::setw(widths["Time"]) << infos[i].cpuTime;
        wcout << " ";
        wcout << std::left << std::setw(widths["Title"]) << infos[i].title;
        wcout << endl;
    }
}