#ifndef DISCORDRPC_H
#define DISCORDRPC_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "socketconnection.h"
#include "discordeventhandlers.h"
#include "discordactivity.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SentHandshake = 0,
    Connected = 1,
    Disconnected = 2
} SocketState;

typedef struct {
    SocketConnection socket;
    SocketState state;
    DiscordUser user;
    bool connected;
    char* client_id;
    char* last_error;
    pthread_t read_thread;
    pthread_t message_thread;
    DiscordEventHandlers handlers;
    MessageQueue queue;
} DiscordRPC;

void DiscordRPC_init(DiscordRPC* self, const char* client_id, DiscordEventHandlers* handlers);
void DiscordRPC_shutdown(DiscordRPC* self);
void DiscordRPC_setActivity(DiscordRPC* self, DiscordActivity *activity);
void DiscordRPC_sendInviteResponse(DiscordRPC* self, const char* user_id, char* response);
void DiscordRPC_acceptInvite(DiscordRPC* self, const char* user_id);
void DiscordRPC_declineInvite(DiscordRPC* self, const char* user_id);
bool DiscordRPC_openSocket(DiscordRPC* self);
void DiscordRPC_onDisconnect(SocketConnection* socket, void* data);
bool DiscordRPC_sendHandshake(DiscordRPC* self);
void DiscordRPC_initHandlers(DiscordRPC* self);
bool DiscordRPC_readMessage(DiscordRPC* self, MessageFrame* frame);
bool DiscordRPC_read(DiscordRPC* self, void* buffer, size_t size);
bool DiscordRPC_writeMessage(DiscordRPC* self, MessageFrame* frame);
bool DiscordRPC_write(DiscordRPC* self, void* data, size_t size);
void* DiscordRPC_readThread(void* arg);
void* DiscordRPC_messageProcessor(void* arg);
const char* DiscordRPC_getTempPath(void);

#ifdef __cplusplus
}
#endif

#endif // DISCORDRPC_H
