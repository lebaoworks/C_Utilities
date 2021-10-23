#define _GNU_SOURCE
#include "unix_socket.h"

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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
static bool _connect(int fd, char *endpoint) {
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
 * @param endpoint null-terminated string. (endpoint[0]=0 to use abstract socket - not available in macOS)
 * @param n maximum number of queued connections
 * 
 * @return true if success, false otherwise
 */
bool listen_on(int fd, char *endpoint, int n) {
    struct sockaddr_un addr;
    int len;

#ifndef __APPLE__
    if (endpoint[0] == 0)
        len = strlen(endpoint+1) + 1;
    else
#endif
        len = strlen(endpoint);
    if (len > sizeof(addr.sun_path) - 1)    // preserve 1 byte for null
        return false;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, endpoint, len);
    
    if (endpoint[0] != 0 && unlink(endpoint) == -1)     // remove old file
        return false;

    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
        return false;
    
    if (listen(fd, n) == -1)
        return false;
    return true;
}

/**
 * @brief Listen on endpoint, and protect endpoint from deletion by
 *      set its parent directory's permission to non-writable
 *      -> no create, rename or delete file in the directory

 * @param fd file descriptor for the socket
 * @param endpoint endpoint address represents as a null-terminated string. Must not be in root dir (/)
 * @param n maximum number of queued connections
 * 
 * @return true if success, false otherwise
 * 
 * @note race condition delete socket file (the same user) between listen_on() and chmod()
 */
bool listen_on_safe(int fd, UDSocketEndpoint *endpoint, int n)
{
    if (chmod(endpoint->parent, 01700) == -1)
    {
        perror("chmod write fail");
        return false;
    }
    
    if (listen_on(fd, endpoint->path, n))
    {
        if (endpoint->path[0] != 0) // abstract need no protection
            if (chmod(endpoint->parent, 01555) == -1)
            {
                close(fd);
                return false;
            }
        return true;
    }   
    return false;
}

/**
 * @brief Service
 * 
 * @param fd file descriptor for the socket
 * @param stop condition for loop, set true to stop accepting connection
 * @param func call this function to handle connection,
 *      this function must close the socket
 */
void service(int fd, bool *stop, UDSocketDispatcher func)
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
 * @brief Read until got n bytes or connection closed
 * 
 * @param fd file descriptor for the socket
 * @param buffer store data received
 * @param n maximum number of bytes to get
 */
int recvn(int fd, char *buffer, int n)
{
    int nread, save = n;
    while (n > 0)
        if ((nread = recv(fd, buffer, n, 0)) == -1)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue;
        }
        else
            if (nread == 0)
                break;
            else
            {
                n -= nread;
                buffer += nread;
            }
    return save-n;
}

/**
 * @brief Send until sent n bytes, or connection closed
 * 
 * @param fd file descriptor for the socket
 * @param buffer data to send
 * @param n maximum number of bytes to send
 */
int sendn(int fd, char *buffer, int n)
{
    int nwrite, save = n;
    while (n > 0)
        if ((nwrite = send(fd, buffer, n, 0)) == -1)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue;
        }
        else
            if (nwrite == 0)
                break;
            else
            {
                n -= nwrite;
                buffer += nwrite;
            }
    return save-n;
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
    id_t pid;
    socklen_t len = sizeof(pid);
    if (getsockopt(fd, SOL_LOCAL, LOCAL_PEERPID, &pid, &len) == -1)
        return;
    peer_cred->pid = pid;
    #endif
}

_UDSocket const UDSocket = {
    .create = create,
    .connect = _connect,
    .listen_on = listen_on,
    .listen_on_safe = listen_on_safe,
    .service = service,
    .close = close,

    .sendn = sendn,
    .recvn = recvn,

    .get_cred = get_cred,
};
