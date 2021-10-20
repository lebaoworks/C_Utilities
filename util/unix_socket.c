#define _GNU_SOURCE
#include "unix_socket.h"

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/un.h>
#include <sys/socket.h>

/**
 * @brief Create new unix domain socket.
 * 
 * @return file descriptor for new socket, -1 if failed
 */
static int create() {
    int fd;
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;
    return fd;
}

/**
 * @brief Connect to endpoint
 * 
 * @param fd file descriptor for the socket
 * @param endpoint endpoint address represents as a null-terminated string
 * 
 * @return true if success, false otherwise
 */
static bool _connect(int fd, char* endpoint) {
    int len = 0;
    if ((len = strlen(endpoint)) > 107)
        return false;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, endpoint, len);

    if (connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        printf("error: %d\n", errno);
        return false;
    }
    return true;
}

/**
 * @brief Bind to endpoint
 * 
 * @param fd file descriptor for the socket
 * @param endpoint endpoint address represents as a null-terminated string
 * 
 * @return true if success, false otherwise
 */
static bool _bind(int fd, char* endpoint) {
    int len = strlen(endpoint);
    if (len > 107)
        return false;
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, endpoint, len);
    
    // assert permission
    if (remove(endpoint) == -1 && errno != ENOENT)
        return false;

    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
        return false;
    return true;
}

/**
 * @brief Listen on the endpoint
 * 
 * @param fd file descriptor for the socket
 * @param n maximum number of queued connections
 * 
 * @return true if success, false otherwise
 */
static bool _listen(int fd, int n) {
    if (listen(fd, n) == -1)
        return false;
    return true;
}

/**
 * @brief Listen on the endpoint
 * 
 * @param stop pointer to file descriptor for the socket
 * @param n maximum number of queued connections
 * 
 * @return true if success, false otherwise
 */
// static void service(bool* stop, HandleFunc func, bool block);


_UnixDomainSocket const UnixDomainSocket = {
    create,
    .connect = _connect,
    .bind = _bind,
    .listen = _listen,
};
