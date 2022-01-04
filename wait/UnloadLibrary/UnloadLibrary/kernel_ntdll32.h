#ifndef _KERNEL_NTDLL32_
#define _KERNEL_NTDLL32_

#include <Windows.h>
#include "kernel_ntdef.h"

typedef ULONG PTR32, HANDLE32, SIZE_T32;

typedef struct _PEB_LDR_DATA32 {
    ULONG       Length;
    BOOLEAN     Initialized;
    HANDLE32       SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    PTR32 EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE32 ShutdownThreadId;
} PEB_LDR_DATA32;

#define GDI_HANDLE_BUFFER_SIZE32 34
typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];

typedef struct _RTL_USER_PROCESS_PARAMETERS32 {
    BYTE Reserved1[16];
    PTR32 Reserved2[10];
    UNICODE_STRING32 ImagePathName;
    UNICODE_STRING32 CommandLine;
} RTL_USER_PROCESS_PARAMETERS32;

typedef struct _PEB32
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union
    {
        BOOLEAN BitField;
        struct
        {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN IsProtectedProcessLight : 1;
            BOOLEAN SpareBits : 1;
        };
    };
    HANDLE32 Mutant;

    PTR32 ImageBaseAddress;
    PTR32 Ldr;               //P->PEB_LDR_DATA32
    PTR32 ProcessParameters; //P->RTL_USER_PROCESS_PARAMETERS
    PTR32 SubSystemData;
    HANDLE32 ProcessHeap;
    PTR32 FastPebLock;
    PTR32 AtlThunkSListPtr;
    PTR32 IFEOKey;
    union
    {
        ULONG CrossProcessFlags;
        struct
        {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ReservedBits0 : 27;
        };
    };
    union
    {
        PTR32 KernelCallbackTable;
        PTR32 UserSharedInfoPtr;
    };
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPtr32;
    PTR32 ApiSetMap;
    ULONG TlsExpansionCounter;
    PTR32 TlsBitmap;
    ULONG TlsBitmapBits[2];
    PTR32 ReadOnlySharedMemoryBase;
    PTR32 HotpatchInformation;
    PTR32 ReadOnlyStaticServerData;
    PTR32 AnsiCodePageData;
    PTR32 OemCodePageData;
    PTR32 UnicodeCaseTableData;

    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T32 HeapSegmentReserve;
    SIZE_T32 HeapSegmentCommit;
    SIZE_T32 HeapDeCommitTotalFreeThreshold;
    SIZE_T32 HeapDeCommitFreeBlockThreshold;

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PTR32 ProcessHeaps;

    PTR32 GdiSharedHandleTable;
    PTR32 ProcessStarterHelper;
    ULONG GdiDCAttributeList;

    PTR32 LoaderLock;

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    PTR32 ImageProcessAffinityMask;
    GDI_HANDLE_BUFFER32 GdiHandleBuffer;
    PTR32 PostProcessInitRoutine;

    PTR32 TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[0x20];

    ULONG SessionId;

    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PTR32 pShimData;
    PTR32 AppCompatInfo;

    UNICODE_STRING32 CSDVersion;

    PTR32 ActivationContextData;
    PTR32 ProcessAssemblyStorageMap;
    PTR32 SystemDefaultActivationContextData;
    PTR32 SystemAssemblyStorageMap;

    SIZE_T32 MinimumStackCommit;

    PTR32* FlsCallback;
    LIST_ENTRY32 FlsListHead;
    PTR32 FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;

    PTR32 WerRegistrationData;
    PTR32 WerShipAssertPtr;
    PTR32 pContextData;
    PTR32 pImageHeaderHash;
    union
    {
        ULONG TracingFlags;
        struct
        {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        };
    };
    ULONGLONG CsrServerReadOnlySharedMemoryBase;
} PEB32;


typedef struct _RTL_BALANCED_NODE32
{
    union
    {
        PTR32 Children[2];
        struct
        {
            PTR32 Left;
            PTR32 Right;
        };
    };
    union
    {
        UCHAR Red : 1;
        UCHAR Balance : 2;
        PTR32 ParentValue;
    };
} RTL_BALANCED_NODE32;

typedef struct _LDR_DATA_TABLE_ENTRY32
{
    LIST_ENTRY32 InLoadOrderLinks;
    LIST_ENTRY32 InMemoryOrderLinks;
    union
    {
        LIST_ENTRY32 InInitializationOrderLinks;
        LIST_ENTRY32 InProgressLinks;
    };
    PTR32 DllBase;
    PTR32 EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING32 FullDllName;
    UNICODE_STRING32 BaseDllName;
    union
    {
        UCHAR FlagGroup[4];
        ULONG Flags;
        struct
        {
            ULONG PackagedBinary : 1;
            ULONG MarkedForRemoval : 1;
            ULONG ImageDll : 1;
            ULONG LoadNotificationsSent : 1;
            ULONG TelemetryEntryProcessed : 1;
            ULONG ProcessStaticImport : 1;
            ULONG InLegacyLists : 1;
            ULONG InIndexes : 1;
            ULONG ShimDll : 1;
            ULONG InExceptionTable : 1;
            ULONG ReservedFlags1 : 2;
            ULONG LoadInProgress : 1;
            ULONG LoadConfigProcessed : 1;
            ULONG EntryProcessed : 1;
            ULONG ProtectDelayLoad : 1;
            ULONG ReservedFlags3 : 2;
            ULONG DontCallForThreads : 1;
            ULONG ProcessAttachCalled : 1;
            ULONG ProcessAttachFailed : 1;
            ULONG CorDeferredValidate : 1;
            ULONG CorImage : 1;
            ULONG DontRelocate : 1;
            ULONG CorILOnly : 1;
            ULONG ReservedFlags5 : 3;
            ULONG Redirected : 1;
            ULONG ReservedFlags6 : 2;
            ULONG CompatDatabaseProcessed : 1;
        };
    };
    USHORT ObsoleteLoadCount;
    USHORT TlsIndex;
    LIST_ENTRY32 HashLinks;
    ULONG TimeDateStamp;
    PTR32 EntryPointActivationContext;
    PTR32 Lock;
    PTR32 DdagNode;
    LIST_ENTRY32 NodeModuleLink;
    PTR32 LoadContext;
    PTR32 ParentDllBase;
    PTR32 SwitchBackContext;
    RTL_BALANCED_NODE32 BaseAddressIndexNode;
    RTL_BALANCED_NODE32 MappingInfoIndexNode;
    PTR32 OriginalBase;
    LARGE_INTEGER LoadTime;
    ULONG BaseNameHashValue;
    LDR_DLL_LOAD_REASON LoadReason;
    ULONG ImplicitPathOptions;
    ULONG ReferenceCount;
} LDR_DATA_TABLE_ENTRY32;

#endif