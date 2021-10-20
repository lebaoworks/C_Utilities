#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "util/unix_socket.h"

int main()
{
    int fd = UnixDomainSocket.create();
    if (!UnixDomainSocket.bind(fd, "uds_server"))  {
        printf("Failed to bind. %d", errno);
        return 1;
    }

    if (!UnixDomainSocket.listen(fd, 5))  {
        printf("Failed to listen.");
        return 1;
    }

    int cfd = accept(fd, NULL, NULL);

    if (cfd != -1) {
        printf("connect fd: %d\n", cfd);
       
        struct ucred ucred;
        int len = sizeof(struct ucred);

        if (getsockopt(cfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1) {
            printf("Get opt failed: %d", errno);
            return 0;
        }

        printf("Credentials from SO_PEERCRED: pid=%ld, euid=%ld, egid=%ld\n",
        (long) ucred.pid, (long) ucred.uid, (long) ucred.gid);

        close(cfd);
    }
    while (1)
    {
        
    }
    return 0;
}