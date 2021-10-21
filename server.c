#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "util/unix_socket.h"

void authenticate(int fd)
{
    UDSocketCred cre;
    UDSocket.get_cred(fd, &cre);
    printf("[+] pid: %d\n", cre.pid);
    close(fd);
}

int main()
{
    int fd = UDSocket.create();
    if (!UDSocket.listen_on(fd, "uds_server", 5))  {
        printf("Failed to listen_on. %d", errno);
        return 1;
    }

    bool stop=false;
    UDSocket.service(fd, &stop, authenticate);
    return 0;
}