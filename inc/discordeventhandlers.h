#ifndef DISCORD_EVENTHANDLERS_H
#define DISCORD_EVENTHANDLERS_H
#define DISCORD_PREMIUM_NONE 0
#define DISCORD_PREMIUM_CLASSIC 1
#define DISCORD_PREMIUM_NITRO 2
#define DISCORD_PREMIUM_NITRO_BASIC 3

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* id;
    const char* username;
    __attribute__((deprecated("Discriminator is deprecated, use username instead"))) const char* discriminator;
    const char* global_name;
    const char* avatar;
    bool bot;
    uint32_t flags;
    uint8_t premium_type;
} DiscordUser;

typedef struct {
    void (*ready)(const DiscordUser* user);
    void (*disconnected)(const bool was_error);
    void (*error)(int error_code, const char* message);
    void (*joinGame)(const char* join_secret);
    void (*spectateGame)(const char* spectate_secret);
    void (*joinRequest)(const DiscordUser* user);
} DiscordEventHandlers;

#ifdef __cplusplus
}
#endif

#endif // DISCORD_EVENTHANDLERS_H
