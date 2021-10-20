#pragma once

#define _GNU_SOURCE

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef void (*HandleFunc)(int fd);

typedef struct {
    int     (*const create)     ();
    bool    (*const connect)    (int fd, char* endpoint);
    bool    (*const bind)       (int fd, char* endpoint);
    bool    (*const listen)     (int fd, int n);
    // void    (*const service)    (bool* stop, HandleFunc func, bool block);
    // void    (*const close)      (int fd);

} _UnixDomainSocket;

extern _UnixDomainSocket const UnixDomainSocket;
