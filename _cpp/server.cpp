#include <stdio.h>
#include "util/ipc.h"
#include "ipc_shared.h"

// void authenticate(int fd)
// {
//     IPCCredential cre;
//     IPC.get_cred(fd, &cre);
//     printf("[+] pid: %lu\n", cre.pid);
//     close(fd);
// }

int main()
{
    IPCEndpoint endpoint = {
        .path = IPC_ENDPOINT,
        .n = 5,
        .handle = server_handle,
        .stop = false,
    };

    printf("bind: %d\n", IPC.bind(&endpoint));
    IPC.serve(&endpoint);
    return 0;
}