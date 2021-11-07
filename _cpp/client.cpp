#include <stdio.h>
#include <stdlib.h>
#include "util/ipc.h"
#include "ipc_shared.h"

int main()
{
    IPCEndpoint endpoint = {
        .path = IPC_ENDPOINT
    };
    IPCConnect *connect = IPC.connect(&endpoint);
    printf("connect: %p\n", connect);
    client_handle(connect);
    return 0;
}