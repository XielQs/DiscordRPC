# DiscordRPC

Simple and easy to use Discord Rich Presence library for C.

Official library is deprecated and not maintained anymore. So I decided to create a new one, but with some improvements and bug fixes.

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

1. Clone repo

```bash
git clone https://github.com/XielQs/DiscordRPC --recursive
cd DiscordRPC
```

2. Build

If you want to build a static library, you can do it by running:

```bash
make lib -j
```

If you want to build a shared library, you can do it by running:

```bash
make shared -j
```

## Usage

You need to link the library with your project. You can do this by adding `-ldiscordrpc` and `-ljansson` to your compiler flags.

```c
#include <discordrpc.h>
#include <stdio.h>

int main() {
    // Initialize Discord RPC
    DiscordRPC discord;
    memset(&discord, 0, sizeof(discord));
    DiscordRPC_init(&discord, "YOUR_CLIENT_ID", NULL);
    if (discord.connected) {
        // Set the presence
        DiscordActivity activity;
        memset(&activity, 0, sizeof(activity));
        activity.state = "Example State";
        activity.details = "Example Details";
        activity.startTimestamp = time(NULL);

        DiscordRPC_setActivity(&discord, activity);
        // Run the main loop or do other stuff
        DiscordRPC_shutdown(&discord); // Shutdown Discord RPC
    }
}
```

## Known Issues

- There is no way to handle events yet, so you can't handle the `onReady` and `onDisconnected` events. You can only check if the connection is established or not.

## API Reference

It is nearly the same as the official library, you can find the official documentation [here](https://discord.com/developers/docs/rich-presence/using-with-the-game-sdk#updating-presence).

But if you want an overview of the API, here it is:

- `DiscordRPC_init(DiscordRPC* discord, const char* clientId, DiscordEventHandlers* handlers)` - Initialize Discord RPC
  - `discord` - Pointer to the DiscordRPC struct
  - `clientId` - Your Discord application client ID
  - `handlers` - Pointer to the DiscordEventHandlers struct (not implemented yet, pass NULL)
- `DiscordRPC_shutdown(DiscordRPC* discord)` - Shutdown Discord RPC
  - `discord` - Pointer to the DiscordRPC struct
- `DiscordRPC_setActivity(DiscordRPC* discord, DiscordActivity activity)` - Set the activity
  - `discord` - Pointer to the DiscordRPC struct
  - `activity` - DiscordActivity struct containing the activity data (pass NULL to clear the activity)
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

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue if you find any bugs or have suggestions for improvements.

My code sucks, so feel free to suggest any improvements or fixes. I will try to implement them as soon as possible.
