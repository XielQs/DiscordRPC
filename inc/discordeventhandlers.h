#ifndef DISCORD_EVENTHANDLERS_H
#define DISCORD_EVENTHANDLERS_H
#include <stdint.h>

typedef struct {
    const char* userId;
    const char* username;
    const char* discriminator;
    const char* avatar;
} DiscordUser;

typedef struct {
    void (*ready)(const DiscordUser* request);
    void (*disconnected)(int errorCode, const char* message);
    void (*errored)(int errorCode, const char* message);
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
    void (*joinRequest)(const DiscordUser* request);
} DiscordEventHandlers;

#endif // DISCORD_EVENTHANDLERS_H
