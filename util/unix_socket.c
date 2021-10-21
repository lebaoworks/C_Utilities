#define _GNU_SOURCE
#include "unix_socket.h"

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


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
    struct sockaddr_un addr;
    int len = 0;
    if ((len = strlen(endpoint)) > sizeof(addr.sun_path))
        return false;

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
 * @brief Listen on endpoint
 * 
 * @param fd file descriptor for the socket
 * @param endpoint endpoint address represents as a null-terminated string
 * @param n maximum number of queued connections
 * 
 * @return true if success, false otherwise
 */
bool listen_on(int fd, char* endpoint, int n) {
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
    
    if (listen(fd, n) == -1)
        return false;
    return true;
}

/**
 * @brief Service
 * 
 * @param fd file descriptor for the socket
 * @param stop condition for loop, set true to stop accepting connection
 * @param func call this function to handle connection,
 *      this function must close the socket
 */
void service(int fd, bool* stop, UDSocketDispatcher func)
{
    int cfd;
    while (!*stop)
    {
        cfd = accept(fd, NULL, NULL);
        if (cfd != -1)
            func(cfd);
    }
}

/**
 * @brief Get peer credentials
 * 
 * @param fd file descriptor for peer's connection
 * @param peer_cred save credentials, default value for every field is 0
 */
static void get_cred(int fd, UDSocketCred* peer_cred)
{
    memset(peer_cred, 0, sizeof(UDSocketCred));
    #ifdef __linux__
    struct ucred ucred;
    unsigned int len = sizeof(struct ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        return;
    peer_cred->pid = ucred.pid;
    peer_cred->uid = ucred.uid;
    peer_cred->gid = ucred.gid;
    #endif
    #ifdef __APPLE__
    #endif
}
_UDSocket const UDSocket = {
    create,
    .connect = _connect,
    listen_on,
    service,
    close,

    get_cred,
};
