#include "queue.h"
#include <string.h>

void Queue_init(MessageQueue* queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
}

bool Queue_isEmpty(MessageQueue* queue) {
    return queue->size == 0;
}

bool Queue_isFull(MessageQueue* queue) {
    return queue->size == QUEUE_CAPACITY;
}

bool Queue_enqueue(MessageQueue* queue, const MessageFrame* message) {
    if (Queue_isFull(queue)) {
        return false;
    }

    queue->rear = (queue->rear + 1) % QUEUE_CAPACITY;
    memcpy(&queue->messages[queue->rear], message, sizeof(MessageFrame));
    queue->size++;
    return true;
}

bool Queue_dequeue(MessageQueue* queue, MessageFrame* message) {
    if (Queue_isEmpty(queue)) {
        return false;
    }

    memcpy(message, &queue->messages[queue->front], sizeof(MessageFrame));
    queue->front = (queue->front + 1) % QUEUE_CAPACITY;
    queue->size--;
    return true;
}
