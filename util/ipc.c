#define _GNU_SOURCE
#include "ipc.h"
#include "os.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int new_fd()
{
    int fd;
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;
    return fd;
}

/**
 * @brief Send until sent n bytes, or connection closed
 * 
 * @param fd file descriptor for the socket
 * @param buffer data to send
 * @param n maximum number of bytes to send
 * 
 * @return number of bytes sent
 */
static size_t sendn(int fd, void *buffer, size_t n)
{
    int nwrite, save = n;
    while (n > 0)
        if ((nwrite = send(fd, buffer, n, MSG_NOSIGNAL)) == -1)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            break;
        }
        else
            if (nwrite == 0)
                break;
            else
            {
                n -= nwrite;
                buffer = (char*)buffer + nwrite;
            }
    return save-n;
}

/**
 * @brief Read until got n bytes or connection closed
 * 
 * @param fd file descriptor for the socket
 * @param buffer store data received
 * @param n maximum number of bytes to get
 * 
 * @return number of bytes received
 */
static size_t recvn(int fd, void *buffer, size_t n)
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
                buffer = (char*) buffer + nread;
            }
    return save-n;
}


/**
 * @brief Bind to endpoint
 * 
 * @param endpoint
 *      [USE]
 *          endpoint->path
 *          endpoint->n
 *      [MOD]
 *          endpoint->fd
 * 
 * @return true if sucess, false otherwise
 */
static bool _bind(IPCEndpoint *endpoint)
{
    char *parent_dir = 0;
    int fd = 0;
    struct stat st;

    // For security purpose
    //      root IPC endpoint must be in /var/.../<parent_dir>/<endpoint>
    //      oths IPC endpoint must be in /tmp/.../<parent_dir>/<endpoint>
    if (geteuid() == 0) {
        if (strncmp(endpoint->path, "/var/", 5) != 0) 
            return false;
    } else {
        if (strncmp(endpoint->path, "/tmp/", 5) != 0)
            return false;
    }
    if (strlen(endpoint->path) > sizeof(((struct sockaddr_un *)0)->sun_path) - 1)    // preserve 1 byte for null
        return false;

    // Prepare path
    parent_dir = dirname(endpoint->path);
    if (parent_dir == NULL || mkdirs(parent_dir, 0) == -1)
        goto FAIL;
    if (stat(parent_dir, &st) == -1)
        goto FAIL;
    if (chmod(parent_dir, st.st_mode | S_IWUSR) ==-1 ||
        (unlink(endpoint->path) == -1 && errno != ENOENT))
        goto FAIL_;

    // Bind
    fd = new_fd();
    if (fd == -1)
        goto FAIL_;
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, endpoint->path);
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1 ||
        listen(fd, endpoint->n) == -1)
        goto FAIL_;

    // Fix permissions
    if (chmod(endpoint->path, 01777) == -1 || chmod(parent_dir, 01555))
        goto FAIL_;
    
    endpoint->fd = fd;
    free(parent_dir);
    return true;

    FAIL_:
    chmod(parent_dir, st.st_mode);

    FAIL:
    close(fd);
    free(parent_dir);
    return false;
}

/**
 * @brief Serve endpoint
 * 
 * @param endpoint
 *      [USE]
 *          endpoint->stop
 *          endpoint->handle
 */
static void _serve(IPCEndpoint *endpoint)
{
    IPCConnect *connect;
    while (endpoint->stop == false)
    {
        connect = IPC.get_connect(endpoint);
        if (connect != NULL)
        {
            endpoint->handle(endpoint, connect);
            IPC.close(connect);
        }
    }
}

/**
 * @brief Wait for a connection
 * 
 * @param endpoint
 *      [USE]
 *          endpoint->fd 
 * @return connection if success, NULL otherwise
 */
static IPCConnect* _get_connect(IPCEndpoint *endpoint)
{
    IPCConnect *connect = (IPCConnect*) calloc(1, sizeof(IPCConnect));
    if (connect == NULL)
        return NULL;
    int cfd = accept(endpoint->fd, NULL, NULL);
    if (cfd == -1)
    {
        free(connect);
        return NULL;
    }
    connect->fd = cfd;
    return connect;
}

/**
 * @brief Connect to endpoint
 * 
 * @param endpoint
 *      [USE]
 *          endpoint->path
 * @return connection if success, NULL otherwise 
 */
static IPCConnect* _connect(IPCEndpoint *endpoint)
{
    if (strlen(endpoint->path) > sizeof(((struct sockaddr_un *)0)->sun_path) - 1)    // preserve 1 byte for null
        return NULL;

    // Connect
    int fd = new_fd();
    if (fd == -1)
        return NULL;

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, endpoint->path);
    if (connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        close(fd);
        return NULL;
    }

    // Alloc return
    IPCConnect *connect = (IPCConnect*) calloc(1, sizeof(IPCConnect));
    if (connect == NULL)
    {
        close(fd);
        return NULL;
    }
    connect->fd = fd;
    return connect;
}

/**
 * @brief Send object to the otherside
 * 
 * @param connect 
 * @param obj 
 * @return true if success, false otherwise
 */
static bool _send(IPCConnect *connect, void *obj)
{
    // Serialize object
    void *data = NULL;
    uint32_t data_size = 0;
    bool alloc = false;
    if (connect->serialize(obj, &data, &data_size, &alloc) == false)
        return false;

    // Send data   
    bool ret = true;
    if (sendn(connect->fd, (void*) &data_size, sizeof(uint32_t)) == sizeof(uint32_t))
        ret = sendn(connect->fd, (char*) data, data_size) == data_size;
    if (alloc)
        free(data);
    return ret;
}

/**
 * @brief Receive object from the otherside
 * 
 * @param connect 
 * @return obj if true, NULL otherwise
 */
static void* _recv(IPCConnect *connect)
{
    // Recv data
    void *data = NULL;
    uint32_t data_size = 0;
    bool alloc = true;
    if (recvn(connect->fd, (char*) &data_size, sizeof(uint32_t)) != sizeof(uint32_t))
        return NULL;
    if ((data = malloc(data_size)) == NULL)
        return NULL;
    if (recvn(connect->fd, data, data_size) != data_size)
    {
        free(data);
        return NULL;
    }
    
    // Deserialize data
    void* obj = connect->deserialize(data, data_size, &alloc);
    if (obj == NULL)
    {
        free(data);
        return NULL;
    }
    if (alloc)
        free(data);

    return obj;   
}

/**
 * @brief Close connection and free memory
 * 
 * @param connect 
 */
static void _close(IPCConnect* connect)
{
    close(connect->fd);
    free(connect);
}

/**
 * @brief Get otherside's credentials
 * 
 * @param connect 
 * @return credentials if success, NULL otherwise 
 */
static IPCCredential* _get_cred(IPCConnect *connect)
{
    IPCCredential *cred = (IPCCredential*) calloc(1, sizeof(IPCCredential));
    if (cred == NULL)
        return NULL;

    #ifdef __linux__
    struct ucred ucred;
    unsigned int len = sizeof(struct ucred);
    if (getsockopt(connect->fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        goto FAIL;
    cred->pid = ucred.pid;
    cred->uid = ucred.uid;
    cred->gid = ucred.gid;
    #endif
    #ifdef __APPLE__
    id_t pid;
    socklen_t len = sizeof(pid);
    if (getsockopt(fd, SOL_LOCAL, LOCAL_PEERPID, &pid, &len) == -1)
        goto FAIL;
    cred->pid = pid;
    #endif
    return cred;

    FAIL:
    free(cred);
    return NULL;
}

_IPC const IPC = {
    .bind = _bind,
    .serve = _serve,
    .get_connect = _get_connect,
    .connect = _connect,

    .send = _send,
    .recv = _recv,
    .close = _close,

    .get_cred = _get_cred,
};
