#include <stdio.h>
#include <errno.h>
#include "util/os.h"
#include "util/ipc.h"

// endpoint : 1777
// parent : 1755

int main()
{
    char path[] = "/tmp/test_ipc/endpoint";
    IPCEndpoint endpoint = {
        .fd = 0,
        .path = path,
        .n = 5,
        .handle = NULL
    };
    
    printf("bind: %d\n", IPC.bind(&endpoint));
    while (true)
        printf("");
    return 0;
}