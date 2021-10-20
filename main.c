#include <stdio.h>
#include "util/queue.h"

int main()
{
    printf("asd\n");

    QueueHandle *handle = Queue.create(3);
    int a[] = {1,2,3,4,5};
    for (int i=0; i<5; i++)
        Queue.push(handle, &a[i]);
    printf("%d\n", handle->count);

    while (!Queue.empty(handle))
    {
        int *x = Queue.pop(handle);
        printf("%p\n", x);
    }
    printf("%p", Queue.pop(handle));
    return 0;
}