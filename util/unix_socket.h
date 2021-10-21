#pragma once

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdint.h>

typedef void (*UDSocketDispatcher)(int fd);
typedef struct _UDSocketCred {
    uint64_t pid;
    uint64_t uid;
    uint64_t gid;
} UDSocketCred;

typedef struct {
    int     (*const create)     ();
    bool    (*const connect)    (int fd, char* endpoint);
    bool    (*const listen_on)  (int fd, char* endpoint, int n);
    void    (*const service)    (int fd, bool* stop, UDSocketDispatcher func);
    int     (*const close)      (int fd);

    void    (*const get_cred)   (int fd, UDSocketCred* peer_cred);

} _UDSocket;

extern _UDSocket const UDSocket;
