#include <stdio.h>
#include "util/queue.h"
#include "util/unix_socket.h"

#define endpoint "/var/tmp/uds_socket/socket"
#define sock_dir "/var/tmp/uds_socket"

int main()
{
    int fd = UDSocket.create();
    if (!UDSocket.connect(fd, endpoint))
        printf("Failed to connect.");
    return 0;
}