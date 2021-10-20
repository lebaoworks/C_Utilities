#include <stdio.h>
#include "util/queue.h"
#include "util/unix_socket.h"

int main()
{
    int fd = UnixDomainSocket.create();
    if (!UnixDomainSocket.connect(fd, "uds_server"))
        printf("Failed to connect.");
    return 0;
}