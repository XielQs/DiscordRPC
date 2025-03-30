#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include "discordrpc.h"
#include <string.h>
#include <jansson.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void DiscordRPC_init(DiscordRPC* self, const char* clientId, DiscordEventHandlers* handlers) {
    if (self == NULL) {
        self->lastError = "DiscordRPC instance is NULL.";
        return;
    }
    if (clientId == NULL) {
        self->lastError = "Client ID is NULL.";
        return;
    }
    if (handlers != NULL) {
        self->handlers = *handlers;
    } else {
        memset(&self->handlers, 0, sizeof(DiscordEventHandlers));
    }

    memset(self, 0, sizeof(DiscordRPC));
    self->clientId = strdup(clientId);
    if (self->clientId == NULL) {
        self->lastError = "Client ID memory allocation failed.";
        return;
    }

    self->connected = false;
    Queue_init(&self->queue);
    if (!DiscordRPC_openSocket(self)) {
        self->lastError = "Socket connection failed.";
        return;
    }
}

void DiscordRPC_setActivity(DiscordRPC *self, DiscordActivity activity) {
    if (!self->connected) {
        self->lastError = "Connection is not established.";
        return;
    }

    json_t *json = json_object();
    if (!json) {
        self->lastError = "JSON object creation failed.";
        return;
    }
    // parse json
    json_object_set_new(json, "cmd", json_string("SET_ACTIVITY"));
    json_t *args = json_object();
    json_object_set_new(args, "pid", json_integer(getpid()));
    json_t *activity_json = json_object();
    json_object_set_new(activity_json, "state", json_string(activity.state));
    json_object_set_new(activity_json, "details", json_string(activity.details));
    json_t *timestamps = json_object();
    json_t *assets = json_object();
    json_t *party = json_object();
    json_t *party_size = json_array();
    json_t *secrets = json_object();
    if (activity.startTimestamp) {
        json_object_set_new(timestamps, "start", json_integer(activity.startTimestamp));
    }
    if (activity.endTimestamp) {
        json_object_set_new(timestamps, "end", json_integer(activity.endTimestamp));
    }
    if (activity.largeImageKey) {
        json_object_set_new(assets, "large_image", json_string(activity.largeImageKey));
    }
    if (activity.largeImageText) {
        json_object_set_new(assets, "large_text", json_string(activity.largeImageText));
    }
    if (activity.smallImageKey) {
        json_object_set_new(assets, "small_image", json_string(activity.smallImageKey));
    }
    if (activity.smallImageText) {
        json_object_set_new(assets, "small_text", json_string(activity.smallImageText));
    }
    if (activity.partyId) {
        json_object_set_new(party, "id", json_string(activity.partyId));
    }
    if (activity.partySize || activity.partyMax) {
        json_array_append_new(party_size, json_integer(activity.partySize));
        json_array_append_new(party_size, json_integer(activity.partyMax));
        json_object_set_new(party, "size", party_size);
    }
    if (activity.partyPrivacy) {
        json_object_set_new(party, "privacy", json_integer(activity.partyPrivacy));
    }
    if (activity.matchSecret) {
        json_object_set_new(secrets, "match", json_string(activity.matchSecret));
    }
    if (activity.joinSecret) {
        json_object_set_new(secrets, "join", json_string(activity.joinSecret));
    }
    if (activity.spectateSecret) {
        json_object_set_new(secrets, "spectate", json_string(activity.spectateSecret));
    }
    json_object_set_new(activity_json, "assets", assets);
    json_object_set_new(activity_json, "party", party);
    json_object_set_new(activity_json, "secrets", secrets);
    json_object_set_new(activity_json, "timestamps", timestamps);
    json_object_set_new(args, "activity", activity_json);
    json_object_set_new(args, "instance", activity.instance ? json_true() : json_false());
    json_object_set_new(json, "args", args);
    json_object_set_new(json, "nonce", json_integer(self->socket.nonce));
    self->socket.nonce++;
    char *json_data = json_dumps(json, JSON_COMPACT);
    if (!json_data) {
        self->lastError = "JSON data creation failed.";
        return;
    }
    json_decref(json);
    size_t json_length = strlen(json_data);
    if (json_length > MaxRpcFrameSize - sizeof(MessageFrameHeader)) {
        self->lastError = "JSON data is too large.";
        free(json_data);
        return;
    }

    MessageFrame sendFrame;
    sendFrame.header.opcode = Frame;
    sendFrame.header.length = (uint32_t)strlen(json_data);
    memcpy(sendFrame.message, json_data, sendFrame.header.length);

    if (!Queue_enqueue(&self->queue, &sendFrame)) {
        self->lastError = "Queue enqueue failed.";
        free(json_data);
        return;
    }
}

bool DiscordRPC_openSocket(DiscordRPC* self) {
    const char* tempPath = DiscordRPC_getTempPath();
    if (tempPath) {
        for (size_t pipeIndex = 0; pipeIndex < 10; ++pipeIndex) {
            char ipc_path[256];
            snprintf(ipc_path, sizeof(ipc_path), "%s/discord-ipc-%zu", tempPath, pipeIndex);
            if (SocketConnection_init(&self->socket, ipc_path)) {
                self->connected = true;
                DiscordRPC_initHandlers(self);
                pthread_create(&self->readThread, NULL, DiscordRPC_readThread, self);
                DiscordRPC_sendHandshake(self);
                pthread_create(&self->messageThread, NULL, DiscordRPC_messageProcessor, self);
                return true;
            }

            self->lastError = "Socket connection failed.";
        }

        return false;
    } else {
        self->lastError = "Temporary path not found.";
    }
    return false;
}

void DiscordRPC_initHandlers(DiscordRPC* self) {
    if (self == NULL) {
        self->lastError = "DiscordRPC instance is NULL.";
        return;
    }
    // todo: initialize handlers
}

bool DiscordRPC_sendHandshake(DiscordRPC* self) {
    if (!self->connected) {
        self->lastError = "Connection is not established.";
        return false;
    }

    json_t *json = json_object();
    if (!json) {
        self->lastError = "JSON object creation failed.";
        return false;
    }
    json_object_set_new(json, "v", json_integer(1));
    json_object_set_new(json, "client_id", json_string(self->clientId));
    char *json_data = json_dumps(json, JSON_COMPACT);
    if (!json_data) {
        self->lastError = "JSON data creation failed.";
        json_decref(json);
        return false;
    }
    json_decref(json);
    MessageFrame sendFrame;
    sendFrame.header.opcode = Handshake;
    sendFrame.header.length = (uint32_t)strlen(json_data);
    memcpy(sendFrame.message, json_data, sendFrame.header.length);

    bool success = DiscordRPC_writeMessage(self, &sendFrame);
    if (!success) {
        self->lastError = "Handshake message sending failed.";
        return false;
    }

    return true;
}

bool DiscordRPC_read(DiscordRPC* self, void* buffer, size_t size) {
    if (!self->connected) {
        self->lastError = "Connection is not established.";
        return false;
    }

    size_t bytesRead = 0;
    while (bytesRead < size) {
        size_t chunkSize = size - bytesRead;
        bool success = SocketConnection_read(&self->socket, (char*)buffer + bytesRead, chunkSize);

        if (!success) {
            self->lastError = "Data reading failed.";
            DiscordRPC_shutdown(self);
            return false;
        }

        bytesRead += chunkSize;
    }

    return true;
}

bool DiscordRPC_readMessage(DiscordRPC* self, MessageFrame* frame) {
    if (!self->connected) {
        self->lastError = "Connection is not established.";
        return false;
    }

    bool success = DiscordRPC_read(self, &frame->header, sizeof(MessageFrameHeader));
    if (!success) {
        self->lastError = "Message header reading failed.";
        return false;
    }

    size_t messageSize = frame->header.length;
    if (messageSize > (MaxRpcFrameSize - sizeof(MessageFrameHeader))) {
        self->lastError = "Message size exceeds maximum limit.";
        return false;
    }

    success = DiscordRPC_read(self, frame->message, messageSize);
    if (!success) {
        self->lastError = "Message data reading failed.";
        return false;
    }

    return true;
}

bool DiscordRPC_write(DiscordRPC* self, void* data, size_t size) {
    if (!self->connected) {
        self->lastError = "Connection is not established.";
        return false;
    }

    return SocketConnection_write(&self->socket, data, size);
}

bool DiscordRPC_writeMessage(DiscordRPC* self, MessageFrame* frame) {
    if (!self->connected) {
        self->lastError = "Connection is not established.";
        return false;
    }

    // bool success = DiscordRPC_write(self, &frame->header, sizeof(MessageFrameHeader));
    // if (!success) {
    //     self->lastError = "Message header writing failed.";
    //     return false;
    // }

    // success = DiscordRPC_write(self, frame->message, frame->header.length);
    // if (!success) {
    //     self->lastError = "Message data writing failed.";
    //     return false;
    // }
    bool success = DiscordRPC_write(self, frame, sizeof(MessageFrameHeader) + frame->header.length);
    if (!success) {
        self->lastError = "Message writing failed.";
        return false;
    }

    return true;
}

void DiscordRPC_shutdown(DiscordRPC* self) {
    if (self->connected) {
        SocketConnection_shutdown(&self->socket);
        self->connected = false;
        pthread_join(self->readThread, NULL);
        pthread_join(self->messageThread, NULL);
        free(self->clientId);
    }
}

const char* DiscordRPC_getTempPath(void) {
    const char* env_vars[] = {"XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP"};

    for (size_t i = 0; i < 4; ++i) {
        const char* value = getenv(env_vars[i]);
        if (value)
            return value;
    }

    return "/tmp";
}

void* DiscordRPC_readThread(void* arg) {
    DiscordRPC* self = (DiscordRPC*)arg;
    MessageFrame frame;

    while (self->connected) {
        if (!DiscordRPC_readMessage(self, &frame)) {
            self->lastError = "Message reading failed.";
            continue;
        }
    }

    return NULL;
}

void* DiscordRPC_messageProcessor(void* arg) {
    DiscordRPC* self = (DiscordRPC*)arg;
    MessageFrame frame;

    while (self->connected) {
        if (Queue_isEmpty(&self->queue)) {
            // wait for 50ms
            usleep(50000);
            continue;
        }

        Queue_dequeue(&self->queue, &frame);

        if (frame.header.length > 0) { 
            if (!DiscordRPC_writeMessage(self, &frame)) {
                self->lastError = "Message writing failed.";
            }
        }
    }
    return NULL;
}
