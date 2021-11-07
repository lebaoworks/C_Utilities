#pragma once

#include <pthread.h>
#include <stdbool.h>

typedef struct _QueueHandle QueueHandle;
typedef struct _QueueEntry QueueEntry;

struct _QueueEntry {
    QueueEntry *next;
    void *value;
};

struct _QueueHandle {
    QueueEntry *head;
    QueueEntry *tail;
    volatile unsigned int count;
    unsigned int limit;
    pthread_mutex_t mutex;
};

typedef struct {
    QueueHandle *(*const create)(unsigned int limit);
    void    (*const destroy)    (QueueHandle *handle, bool free_data);
    bool    (*const push)       (QueueHandle *handle, void *addr);
    void*   (*const pop)        (QueueHandle *handle);
    bool    (*const empty)      (QueueHandle *handle);
    bool    (*const full)       (QueueHandle *handle);
} _Queue;

extern _Queue const Queue;
