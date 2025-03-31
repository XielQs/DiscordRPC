# DiscordRPC

Simple and easy to use Discord Rich Presence library for C.

Official library is deprecated and not maintained anymore and it SUCKS. So I decided to create a new one, but with some improvements and bug fixes.

## Features

- Simple and easy to use
- Almost no external dependencies (only `jansson` for JSON parsing)
- Support for multiple instances

## Supported Platforms

I only tested it on Linux amd64, probably it doesn't work on Windows because of the `unix` socket implementation.

## Dependencies

- `jansson` for JSON parsing
- `libc` for system calls

## Installation

### Clone repo

```bash
git clone https://github.com/XielQs/DiscordRPC --recursive
cd DiscordRPC
```

### Build

If you want to build a static library, you can do it by running:

```bash
make lib -j
```

If you want to build a shared library, you can do it by running:

```bash
make shared -j
```

## Usage

You can use the following example to initialize the library and set the activity:

```c
#include <discordrpc.h>
#include <string.h>
#include <stdio.h>

void readyEvent(const DiscordUser* user) {
    printf("Discord RPC is ready. User ID: %s, Username: %s, Global Name: %s\n", user->id, user->username, user->global_name);
}

int main() {
    // Initialize Discord RPC
    printf("Initializing Discord RPC...\n");
    DiscordRPC discord;
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = readyEvent;
    DiscordRPC_init(&discord, "YOUR_CLIENT_ID", &handlers); // You can pass NULL for handlers if you don't need them
    if (discord.connected) {
        printf("Connected to Discord RPC successfully!\n");
        // Set the presence
        DiscordActivity activity;
        memset(&activity, 0, sizeof(activity));
        activity.state = "Example State";
        activity.details = "Example Details";
        activity.startTimestamp = time(NULL);

        DiscordRPC_setActivity(&discord, &activity);
        // Run the main loop or do other stuff
        while (1) {} // Keep the program running
        DiscordRPC_shutdown(&discord); // Shutdown Discord RPC
    } else {
        printf("Failed to connect to Discord RPC: %s\n", discord.lastError);
        return 1;
    }
    return 0;
}
```

## Known Issues

- There is no known issues at the moment, but if you find any, please open an issue on GitHub.

## API Reference

It is nearly the same as the official library, you can find the official documentation [here](https://discord.com/developers/docs/rich-presence/using-with-the-game-sdk#updating-presence).

But if you want an overview of the API, here it is:

- `DiscordRPC_init(DiscordRPC* discord, const char* client_id, DiscordEventHandlers* handlers)` - Initialize Discord RPC
  - `discord` - Pointer to the DiscordRPC struct
  - `client_id` - Your Discord application client ID
  - `handlers` - Pointer to the DiscordEventHandlers struct
- `DiscordRPC_shutdown(DiscordRPC* discord)` - Shutdown Discord RPC
  - `discord` - Pointer to the DiscordRPC struct
- `DiscordRPC_setActivity(DiscordRPC* discord, DiscordActivity* activity)` - Set the activity
  - `discord` - Pointer to the DiscordRPC struct
  - `activity` - Pointer to the DiscordActivity struct containing the activity data (pass NULL to clear the activity)
- `DiscordRPC_acceptInvite(DiscordRPC* discord, const char* user_id)` - Accept an invite from a user
  - `discord` - Pointer to the DiscordRPC struct
  - `user_id` - The ID of the user to accept the invite from
- `DiscordRPC_declineInvite(DiscordRPC* discord, const char* user_id)` - Decline an invite from a user
  - `discord` - Pointer to the DiscordRPC struct
  - `user_id` - The ID of the user to decline the invite from
- `DiscordActivity` - Struct containing the activity data
  - `state` - The state of the activity (e.g. "Playing")
  - `details` - The details of the activity (e.g. "Playing a game")
  - `startTimestamp` - The start timestamp of the activity (e.g. time(NULL))
  - `endTimestamp` - The end timestamp of the activity (e.g. time(NULL) + 3600)
  - `largeImageKey` - The key of the large image to display
  - `largeImageText` - The text to display when hovering over the large image
  - `smallImageKey` - The key of the small image to display
  - `smallImageText` - The text to display when hovering over the small image
  - `partyId` - The ID of the party
  - `partySize` - The size of the party
  - `partyMax` - The maximum size of the party
  - `partyPrivacy` - The privacy level of the party (`DISCORD_PARTY_PUBLIC` for public, `DISCORD_PARTY_PRIVATE` for private)
  - `matchSecret` - The secret to matchmake with the party
  - `joinSecret` - The secret to join the party
  - `spectateSecret` - The secret to spectate the party
  - `instance` - Whether the activity is an instance or not (1 for true, 0 for false)
- `DiscordEventHandlers` - Struct containing the event handlers
  - `ready(const DiscordUser* user)` - Called when Discord RPC is ready
  - `disconnected(const bool was_error)` - Called when Discord RPC is disconnected
  - `error(int error_code, const char* message)` - Called when there is an error
  - `joinGame(const char* join_secret)` - Called when the user joins another users game
  - `spectateGame(const char* spectate_secret)` - Called when the user spectates another users game
  - `joinRequest(const DiscordUser* user)` - Called when there is a pending join request from a user

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue if you find any bugs or have suggestions for improvements.

My code sucks, so feel free to suggest any improvements or fixes. I will try to implement them as soon as possible.
