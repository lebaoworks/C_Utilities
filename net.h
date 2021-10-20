#pragma once
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#if defined(WIN32) || defined(_WIN32)
    #include <winsock2.h>
    #include <Ws2tcpip.h>
    #define close closesocket
    #define MSG_NOSIGNAL 0
#elif defined(linux)
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

//Increase BUFSIZE can make net_forward_socket() crash in allocating stack memory
#define BUFSIZE 4096

//Init for net library
int net_init()
{
    #if defined(WIN32) || defined(_WIN32)
        WSADATA wsaData;
        int iResult = WSAStartup(0x0202, &wsaData);
        if (iResult != 0)
            return -1;
        return 0;
    #endif
}

void net_close()
{
    #ifdef WIN32
        WSACleanup();
    #endif
}

// Convert 4-byte ip address to dotted-string ip
void net_itoa(uint32_t ip, char buffer[16])
{
    memset(buffer, 0, 16);
    inet_ntop(AF_INET, &ip, buffer, 16);
}

// Convert dotted-string ip address to 4-byte ip
uint32_t net_atoi(char* ip)
{
    uint32_t res;
    inet_pton(AF_INET, ip, &res);
    return res;
}

// Read until got n bytes or connection closed
int net_recv(int fd, char *buf, int n)
{
    int nread, save = n;
    while (n > 0)
        if ((nread = recv(fd, buf, n, 0)) == -1)
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
                buf += nread;
            }
    return save-n;
}

// Write until wrote n bytes or connection closed
int net_send(int fd, char *buf, int n)
{
    int nwrite, save = n;
    while (n > 0)
        if ((nwrite = send(fd, buf, n, 0)) == -1)
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
                buf += nwrite;
            }
    return save-n;
}

// Connect to address:port
int net_tcp_connect(char* addr, unsigned short port)
{
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = net_atoi(addr);
    remote.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return -1;
    char _x = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &_x, sizeof(int));
    if (connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0)
    {
        close(fd);
        return -1;
    }
    return fd;
}

// Connect to address:port
int net_tcp_connect_timeout(char* addr, unsigned short port, unsigned int timeout)
{
    struct sockaddr_in remote;
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Error socket(AF_INET, SOCK_STREAM, 0) (%s)\n", strerror(errno));     
        return -1;
    }

    char _x = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &_x, sizeof(int)) == -1)
    {
        fprintf(stderr, "Error setsockopt(..., SO_REUSEADDR) (%s)\n", strerror(errno));     
        return -1;
    }

    // Set non-blocking
    int arg;
    if ((arg = fcntl(fd, F_GETFL, NULL)) < 0)
    { 
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
        return -1;   
    } 

    arg |= O_NONBLOCK; 
    if (fcntl(fd, F_SETFL, arg) < 0)
    { 
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        return -1;
    } 

    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = net_atoi(addr);
    remote.sin_port = htons(port);

    // if (connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0)
    // {
    //     close(fd);
    //     return -1;
    // }
    // return fd;
    int res = connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in));
    if (res < 0)
    {
        if (errno == EINPROGRESS)
        { 
            fprintf(stderr, "EINPROGRESS in connect() - selecting\n"); 
            do
            { 
                tv.tv_sec = 15; 
                tv.tv_usec = 0; 
                FD_ZERO(&myset); 
                FD_SET(soc, &myset); 
                res = select(soc+1, NULL, &myset, NULL, &tv); 
                if (res < 0 && errno != EINTR) { 
                    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
                    exit(0); 
           } 
           else if (res > 0) { 
              // Socket selected for write 
              lon = sizeof(int); 
              if (getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
                 fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
                 exit(0); 
              } 
              // Check the value returned... 
              if (valopt) { 
                 fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt) 
); 
                 exit(0); 
              } 
              break; 
           } 
           else { 
              fprintf(stderr, "Timeout in select() - Cancelling!\n"); 
              exit(0); 
           } 
        } while (1); 
     } 
     else { 
        fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
        exit(0); 
     } 
  } 
  // Set to blocking mode again... 
  if( (arg = fcntl(soc, F_GETFL, NULL)) < 0) { 
     fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
     exit(0); 
  } 
  arg &= (~O_NONBLOCK); 
  if( fcntl(soc, F_SETFL, arg) < 0) { 
     fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
     exit(0); 
  } 
}

// Connect to address:port
int net_udp_connect(char* addr, int port)
{
    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(struct sockaddr_in));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = net_atoi(addr);
    remote.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
        return -1;
    char _x = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &_x, sizeof(int));
    if (connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0)
    {
        close(fd);
        return -1;
    }
    return fd;
}
