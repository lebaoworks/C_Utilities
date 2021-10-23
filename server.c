#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "util/unix_socket.h"

UDSocketEndpoint endpoint = {
    .path = "/var/tmp/uds_socket/socket",
    .parent = "/var/tmp/uds_socket",
};

void authenticate(int fd)
{
    UDSocketCred cre;
    UDSocket.get_cred(fd, &cre);
    printf("[+] pid: %lu\n", cre.pid);
    close(fd);
}

int main()
{
    int fd = UDSocket.create();
    if (!UDSocket.listen_on_safe(fd, &endpoint, 5))  {
        printf("Failed to listen_on. %d", errno);
        return 1;
    }
    

    bool stop=false;
    UDSocket.service(fd, &stop, authenticate);
    return 0;
}