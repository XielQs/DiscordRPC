#ifndef QUEUE_H
#define QUEUE_H
#include "messageframe.h"
#include <stdbool.h>
#define QUEUE_CAPACITY 10

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MessageQueue {
    MessageFrame messages[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
} MessageQueue;

void Queue_init(MessageQueue* queue);
bool Queue_enqueue(MessageQueue* queue, const MessageFrame* message);
bool Queue_dequeue(MessageQueue* queue, MessageFrame* message);
bool Queue_isEmpty(MessageQueue* queue);
bool Queue_isFull(MessageQueue* queue);

#ifdef __cplusplus
}
#endif

#endif // QUEUE_H
