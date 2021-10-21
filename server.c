#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "util/unix_socket.h"

#define endpoint "/var/tmp/uds_socket/socket"
#define sock_dir "/var/tmp/uds_socket"

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
    if (!UDSocket.listen_on(fd, endpoint, 5))  {
        printf("Failed to listen_on. %d", errno);
        return 1;
    }
    system("chmod 555 /var/tmp/uds_socket");

    bool stop=false;
    UDSocket.service(fd, &stop, authenticate);
    return 0;
}