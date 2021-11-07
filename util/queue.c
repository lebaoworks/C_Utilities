#include "queue.h"
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

/**
 * @brief Create new queue.
 * 
 * @param limit maximum number of entries, 0 for unlimited entries.
 * 
 * @return pointer to queue's handle, NULL if failed
 * @note Call destroy() to release memory
 */
static QueueHandle* create(unsigned int limit) {
    QueueHandle *handle = (QueueHandle*) calloc(1, sizeof(QueueHandle));
    if (handle == NULL)
        return NULL;
    if (pthread_mutex_init(&handle->mutex, NULL) != 0)
    {
        free(handle);
        return NULL;
    }
    handle->limit = limit;
    return handle;
}

/**
 * @brief Destroy queue
 * 
 * @param handle queue's handle
 * @param free_data set true to free queue's elements
 */
static void destroy(QueueHandle *handle, bool free_data)
{
    if (free_data == true)
    {
        void* addr;
        while (!Queue.empty(handle))
        {
            addr = Queue.pop(handle);
            free(addr);
        }
    }
    pthread_mutex_destroy(&handle->mutex);
    free(handle);
}

/**
 * @brief push value's address to queue
 * 
 * @param handle queue's handle
 * @param addr address of element
 * 
 * @return true if success, false otherwise
 */
static bool push(QueueHandle *handle, void *addr)
{
    pthread_mutex_lock(&handle->mutex);

    if (handle->limit!=0 && handle->count == handle->limit)
    {
        pthread_mutex_unlock(&handle->mutex);
        return false;
    }
    QueueEntry *entry = (QueueEntry*) malloc(sizeof(QueueEntry));
    if (entry == NULL)
    {
        pthread_mutex_unlock(&handle->mutex);
        return false;
    }
    entry->value = addr;
    entry->next = NULL;

    // append to empty queue
    if (handle->head == NULL)
        handle->head = handle->tail = entry;
    else {
        // connect tail
        handle->tail->next = entry;
        handle->tail = entry;
    }
    handle->count++;

    pthread_mutex_unlock(&handle->mutex);
    return true;
}

/**
 * @brief pop oldest element's address
 * 
 * @param handle queue's handle
 * 
 * @return element's address, NULL if queue is empty
 */
static void* pop(QueueHandle *handle)
{
    pthread_mutex_lock(&handle->mutex);

    if (handle->count == 0)
    {
        pthread_mutex_unlock(&handle->mutex);
        return NULL;
    }
    QueueEntry *head = handle->head;
    void* addr = head->value;
    handle->head = handle->head->next;
    free(head);
    handle->count--;
    
    pthread_mutex_unlock(&handle->mutex);
    return addr;
}

/**
 * @brief check if queue is empty
 * 
 * @param handle queue's handle
 * 
 * @note empty() is not mutexed
 */
static bool empty(QueueHandle *handle)
{
    return handle->count == 0;
}

/**
 * @brief check if queue is full
 * 
 * @param handle queue's handle
 * 
 * @note empty() is not mutexed
 */
static bool full(QueueHandle *handle)
{
    return (handle->limit==0)?false:handle->count==handle->limit;
}

_Queue const Queue = {
    create,
    destroy,
    push,
    pop,
    empty,
    full
};
