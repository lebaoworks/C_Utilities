#ifndef _KERNEL_NTDLL64_
#define _KERNEL_NTDLL64_

#include <Windows.h>

typedef ULONGLONG PTR64, HANDLE64, SIZE_T64;

typedef struct _UNICODE_STRING64 {
    USHORT Length;
    USHORT MaximumLength;
    PTR64  Buffer;
} UNICODE_STRING64;

typedef struct _PEB_LDR_DATA64 {
    ULONG       Length;
    BOOLEAN     Initialized;
    HANDLE64       SsHandle;
    LIST_ENTRY64 InLoadOrderModuleList;
    LIST_ENTRY64 InMemoryOrderModuleList;
    LIST_ENTRY64 InInitializationOrderModuleList;
    PTR64 EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE64 ShutdownThreadId;
} PEB_LDR_DATA64;

#define GDI_HANDLE_BUFFER_SIZE64 60
typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];

typedef struct _RTL_USER_PROCESS_PARAMETERS64 {
    BYTE Reserved1[16];
    PTR64 Reserved2[10];
    UNICODE_STRING64 ImagePathName;
    UNICODE_STRING64 CommandLine;
} RTL_USER_PROCESS_PARAMETERS64;

typedef struct _PEB64
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
    HANDLE64 Mutant;

    PTR64 ImageBaseAddress;
    PTR64 Ldr;               //P->PEB_LDR_DATA64
    PTR64 ProcessParameters; //P->RTL_USER_PROCESS_PARAMETERS64
    PTR64 SubSystemData;
    HANDLE64 ProcessHeap;
    PTR64 FastPebLock;
    PTR64 AtlThunkSListPtr;
    PTR64 IFEOKey;
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
        PTR64 KernelCallbackTable;
        PTR64 UserSharedInfoPtr;
    };
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPTR64;
    PTR64 ApiSetMap;
    ULONG TlsExpansionCounter;
    PTR64 TlsBitmap;
    ULONG TlsBitmapBits[2];
    PTR64 ReadOnlySharedMemoryBase;
    PTR64 HotpatchInformation;
    PTR64 ReadOnlyStaticServerData;
    PTR64 AnsiCodePageData;
    PTR64 OemCodePageData;
    PTR64 UnicodeCaseTableData;

    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T64 HeapSegmentReserve;
    SIZE_T64 HeapSegmentCommit;
    SIZE_T64 HeapDeCommitTotalFreeThreshold;
    SIZE_T64 HeapDeCommitFreeBlockThreshold;

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PTR64 ProcessHeaps;

    PTR64 GdiSharedHandleTable;
    PTR64 ProcessStarterHelper;
    ULONG GdiDCAttributeList;

    PTR64 LoaderLock;

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    PTR64 ImageProcessAffinityMask;
    GDI_HANDLE_BUFFER64 GdiHandleBuffer;
    PTR64 PostProcessInitRoutine;

    PTR64 TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[0x20];

    ULONG SessionId;

    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PTR64 pShimData;
    PTR64 AppCompatInfo;

    UNICODE_STRING64 CSDVersion;

    PTR64 ActivationContextData;
    PTR64 ProcessAssemblyStorageMap;
    PTR64 SystemDefaultActivationContextData;
    PTR64 SystemAssemblyStorageMap;

    SIZE_T64 MinimumStackCommit;

    PTR64* FlsCallback;
    LIST_ENTRY64 FlsListHead;
    PTR64 FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;

    PTR64 WerRegistrationData;
    PTR64 WerShipAssertPtr;
    PTR64 pContextData;
    PTR64 pImageHeaderHash;
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
} PEB64;

typedef struct _RTL_BALANCED_NODE64
{
    union
    {
        PTR64 Children[2];
        struct
        {
            PTR64 Left;
            PTR64 Right;
        };
    };
    union
    {
        UCHAR Red : 1;
        UCHAR Balance : 2;
        PTR64 ParentValue;
    };
} RTL_BALANCED_NODE64;



typedef struct _LDR_DATA_TABLE_ENTRY64
{
    LIST_ENTRY64 InLoadOrderLinks;
    LIST_ENTRY64 InMemoryOrderLinks;
    union
    {
        LIST_ENTRY64 InInitializationOrderLinks;
        LIST_ENTRY64 InProgressLinks;
    };
    PTR64 DllBase;
    PTR64 EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING64 FullDllName;
    UNICODE_STRING64 BaseDllName;
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
    LIST_ENTRY64 HashLinks;
    ULONG TimeDateStamp;
    PTR64 EntryPointActivationContext;
    PTR64 Lock;
    PTR64 DdagNode;
    LIST_ENTRY64 NodeModuleLink;
    PTR64 LoadContext;
    PTR64 ParentDllBase;
    PTR64 SwitchBackContext;
    RTL_BALANCED_NODE64 BaseAddressIndexNode;
    RTL_BALANCED_NODE64 MappingInfoIndexNode;
    PTR64 OriginalBase;
    LARGE_INTEGER LoadTime;
    ULONG BaseNameHashValue;
    LDR_DLL_LOAD_REASON LoadReason;
    ULONG ImplicitPathOptions;
    ULONG ReferenceCount;
} LDR_DATA_TABLE_ENTRY64;

typedef struct _PROCESS_BASIC_INFORMATION64
{
    NTSTATUS ExitStatus;
    PTR64 PebBaseAddress;
    PTR64 AffinityMask;
    LONG BasePriority;
    PTR64 UniqueProcessId;
    PTR64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64, * PPROCESS_BASIC_INFORMATION64;

#endif