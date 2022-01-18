#include "WinSec.h"

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <accctrl.h>
#include <aclapi.h>
#pragma comment(lib, "advapi32.lib")

#include <string>
using namespace std;

#include <sddl.h>
DWORD GetFileOwner(wstring path, USER_INFO& info)
{
    info.SID = L""; info.DomainName = L""; info.Name = L"";

    if (path.length() == 0)
        return ERROR_INVALID_NAME;

    PSID pSidOwner = NULL;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD dwStatus = GetNamedSecurityInfoW(
        path.c_str(),
        SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION,
        &pSidOwner,
        NULL,
        NULL,
        NULL,
        &pSecurityDescriptor
    );
    if (dwStatus)
        return dwStatus;
      
    LPWSTR szSid = NULL;
    if (ConvertSidToStringSidW(pSidOwner, &szSid) == TRUE)
    {
        info.SID = szSid;
        if (szSid != NULL)
            LocalFree(szSid);
    }

    WCHAR szName[256]; DWORD dwNameLen = 256;
    WCHAR szDomain[256]; DWORD dwDomainLen = 256;
    SID_NAME_USE eUse = SidTypeUnknown;
    dwStatus = (LookupAccountSidW(NULL, pSidOwner, szName, &dwNameLen, szDomain, &dwDomainLen, &eUse) == TRUE)?ERROR_SUCCESS:GetLastError();
    LocalFree(pSecurityDescriptor);
    if (dwStatus == 0)
    {
        info.Name = (szName != NULL) ? wstring(szName, dwNameLen) : L"";
        info.DomainName = (szDomain != NULL) ? wstring(szDomain, dwDomainLen) : L"";
    }

    return dwStatus;
}

#pragma comment(lib, "wbemuuid.lib")
#include <iostream>
#include <map>
#include <Wbemidl.h>
#include <comdef.h>
DWORD GetFileOwnerEx(map<wstring, USER_INFO>& info)
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
{
    HRESULT hres;

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

    IEnumWbemClassObject* pEnumerator = NULL;
    for (map<wstring, USER_INFO>::iterator i = info.begin(); i != info.end(); i++)
    {
        i->second = { L"", L"", L"" };
        // Step 6: --------------------------------------------------
        // Use the IWbemServices pointer to make requests of WMI ----
        hres = pSvc->ExecQuery(
            _bstr_t("WQL"),
            _bstr_t((L"ASSOCIATORS OF {Win32_LogicalFileSecuritySetting='" + i->first + L"'} WHERE AssocClass=Win32_LogicalFileOwner ResultRole=Owner").c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator);

        if (FAILED(hres))
           continue;


        // Step 7: -------------------------------------------------
        // Get the data from the query in step 6 -------------------

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);

            if (0 == uReturn)
                break;

            VARIANT vtProp;     VariantInit(&vtProp);

            hr = pclsObj->Get(L"AccountName", 0, &vtProp, 0, 0);
            if (vtProp.bstrVal != NULL)
                i->second.Name = vtProp.bstrVal;
            else
                i->second.Name = L"";

            hr = pclsObj->Get(L"ReferencedDomainName", 0, &vtProp, 0, 0);
            if (vtProp.bstrVal != NULL)
                i->second.DomainName = vtProp.bstrVal;
            else
                i->second.DomainName = L"";

            hr = pclsObj->Get(L"SID", 0, &vtProp, 0, 0);
            if (vtProp.bstrVal != NULL)
                i->second.SID = vtProp.bstrVal;
            else
                i->second.SID = L"";

            VariantClear(&vtProp);
            pclsObj->Release();
        }
    }
    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return 0;
}