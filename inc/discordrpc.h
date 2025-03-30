#ifndef DISCORDRPC_H
#define DISCORDRPC_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "discordeventhandlers.h"
#include "discordactivity.h"
#include "socketconnection.h"
#include "queue.h"

typedef struct {
    SocketConnection socket;
    bool connected;
    char* clientId;
    char* lastError;
    pthread_t readThread;
    pthread_t messageThread;
    DiscordEventHandlers handlers;
    MessageQueue queue;
} DiscordRPC;

void DiscordRPC_init(DiscordRPC* self, const char* clientId, DiscordEventHandlers* handlers);
void DiscordRPC_shutdown(DiscordRPC* self);
void DiscordRPC_setActivity(DiscordRPC* self, DiscordActivity activity);
bool DiscordRPC_openSocket(DiscordRPC* self);
bool DiscordRPC_sendHandshake(DiscordRPC* self);
void DiscordRPC_initHandlers(DiscordRPC* self);
bool DiscordRPC_readMessage(DiscordRPC* self, MessageFrame* frame);
bool DiscordRPC_read(DiscordRPC* self, void* buffer, size_t size);
bool DiscordRPC_writeMessage(DiscordRPC* self, MessageFrame* frame);
bool DiscordRPC_write(DiscordRPC* self, void* data, size_t size);
void* DiscordRPC_readThread(void* arg);
void* DiscordRPC_messageProcessor(void* arg);
const char* DiscordRPC_getTempPath(void);

#endif // DISCORDRPC_H
