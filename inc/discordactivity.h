#ifndef DISCORD_ACTIVITY_H
#define DISCORD_ACTIVITY_H
#define DISCORD_PARTY_PRIVATE 0
#define DISCORD_PARTY_PUBLIC 1

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* state;
    const char* details;
    int64_t startTimestamp;
    int64_t endTimestamp;
    const char* largeImageKey;
    const char* largeImageText;
    const char* smallImageKey;
    const char* smallImageText;
    const char* partyId;
    int partySize;
    int partyMax;
    int partyPrivacy;
    const char* matchSecret;
    const char* joinSecret;
    const char* spectateSecret;
    bool instance;
} DiscordActivity;

#ifdef __cplusplus
}
#endif

#endif // DISCORD_ACTIVITY_H
