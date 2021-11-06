#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define IPC_ENDPOINT "/tmp/tmp/uds_socket/socket"

typedef struct _IPCEndpoint IPCEndpoint;
typedef struct _IPCConnect IPCConnect;

// =================================== //
// ========ABSTRACT FUNCTIONS========= //
// =================================== //

typedef void (*IPCHANDLE)(
    IPCEndpoint*,   // [in] endpoint, do not release this
    IPCConnect*     // [in] new connection, do not release this
);

typedef bool (*IPCSERIALIZE)(
    void*,          // [in] object
    void**,         // [out] serialized data
    uint32_t*,      // [out] serialized data length
    bool*           // [out] if binary is newly allocated, set true to release serialized data
);  // -> true if success, false otherwise

typedef void* (*IPCDESERIALIZE)(
    void*,          // [in] serialized data
    uint32_t,       // [in] serialized data length
    bool*           // [out] if object is newly allocated, set true to release serialized data
);  // -> object if success, NULL otherwise

// ============================= //
// ========BASE STRUCTS========= //
// ============================= //

/**
 * @brief Can make sub-class of this struct by
 *  putting this struct at very first.
 */
struct _IPCEndpoint {
    int fd;             // [internal]
    char *path;         // [in] endpoint
    int n;              // [in] maximum handle
    IPCHANDLE handle;   // [in] handle function
    bool stop;          // set true to stop serve loop
};

/**
 * @brief Can make sub-class of this struct by
 *  putting this struct at very first.
 */
struct _IPCConnect {
    int fd;
    IPCSERIALIZE serialize;
    IPCDESERIALIZE deserialize;
};

typedef struct _IPCCredential {
    uint64_t pid;
    uint64_t uid;
    uint64_t gid;
} IPCCredential;



// =========================== //
// ========NAME SPACE========= //
// =========================== //

typedef struct {
    bool            (*const bind)       (IPCEndpoint *endpoint);
    void            (*const serve)      (IPCEndpoint *endpoint);
    IPCConnect*     (*const get_connect)(IPCEndpoint *endpoint);
    IPCConnect*     (*const connect)    (IPCEndpoint *endpoint);
    
    bool            (*const send)       (IPCConnect *connect, void *obj);
    void*           (*const recv)       (IPCConnect *connect);
    void            (*const close)      (IPCConnect *connect);
    
    IPCCredential*  (*const get_cred)   (IPCConnect *connect);
} _IPC;

extern _IPC const IPC;

