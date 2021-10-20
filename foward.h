#pragma once

#include <errno.h>
#include <sys/select.h>

#include "net/net.h"

// Connect 2 sockets
void net_forward(int fd0, int fd1)
{
    int maxfd, ret;
    fd_set rd_set;
    size_t nread;
    char buffer_r[BUFSIZE];

    maxfd = (fd0 > fd1) ? fd0 : fd1;
    while (1)
    {
        FD_ZERO(&rd_set);
        FD_SET(fd0, &rd_set);
        FD_SET(fd1, &rd_set);
        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR)
            continue;

        if (FD_ISSET(fd0, &rd_set))
        {
            nread = recv(fd0, buffer_r, BUFSIZE, 0);
            if (nread <= 0)
                break;
            send(fd1, buffer_r, nread, 0);
        }

        if (FD_ISSET(fd1, &rd_set))
        {
            nread = recv(fd1, buffer_r, BUFSIZE, 0);
            if (nread <= 0)
                break;
            send(fd0, buffer_r, nread, 0);
        }
    }
}