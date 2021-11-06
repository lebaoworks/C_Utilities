#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "util/ipc.h"

bool serialize(void *obj, void **pdata, uint32_t *data_size, bool *alloc)
{
    *pdata = obj;
    *data_size = strlen((char*) obj);
    *alloc = false;
    return true;
}
void* deserialize(void *obj, uint32_t data_size, bool *alloc)
{
    *alloc = false;
    return obj;
}

void server_handle(IPCEndpoint* endpoint, IPCConnect* connect)
{
    printf("[+] Handle\n");
    connect->serialize = serialize;
    connect->deserialize = deserialize;
    printf("send: %d\n", IPC.send(connect, "asd"));
    endpoint->stop = true;
}

void client_handle(IPCConnect *connect)
{
    connect->deserialize = deserialize;
    connect->serialize = serialize;
    char* obj = (char*) IPC.recv(connect);
    printf("%s\n", obj);
    free(obj);
}