#define _DEFAULT_SOURCE
#include "discordrpc.h"
#include "queue.h"
#include <string.h>
#include <jansson.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void DiscordRPC_init(DiscordRPC* self, const char* client_id, DiscordEventHandlers* handlers) {
    if (self == NULL) {
        self->last_error = "DiscordRPC instance is NULL.";
        return;
    }
    if (client_id == NULL) {
        self->last_error = "Client ID is NULL.";
        return;
    }

    memset(self, 0, sizeof(DiscordRPC));
    if (handlers != NULL) {
        self->handlers = *handlers;
    }
    self->client_id = strdup(client_id);
    if (self->client_id == NULL) {
        self->last_error = "Client ID memory allocation failed.";
        return;
    }

    self->connected = false;
    Queue_init(&self->queue);
    if (!DiscordRPC_openSocket(self)) {
        self->last_error = "Socket connection failed.";
        return;
    }
}

void DiscordRPC_setActivity(DiscordRPC *self, DiscordActivity *activity) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
        return;
    }

    json_t *json = json_object();
    if (!json) {
        self->last_error = "JSON object creation failed.";
        return;
    }
    // parse json
    json_object_set_new(json, "cmd", json_string("SET_ACTIVITY"));
    json_t *args = json_object();
    json_object_set_new(args, "pid", json_integer(getpid()));
    json_t *activity_json = json_object();
    json_object_set_new(activity_json, "state", json_string(activity->state));
    json_object_set_new(activity_json, "details", json_string(activity->details));
    json_t *timestamps = json_object();
    json_t *assets = json_object();
    json_t *party = json_object();
    json_t *party_size = json_array();
    json_t *secrets = json_object();
    if (activity->startTimestamp) {
        json_object_set_new(timestamps, "start", json_integer(activity->startTimestamp));
    }
    if (activity->endTimestamp) {
        json_object_set_new(timestamps, "end", json_integer(activity->endTimestamp));
    }
    if (activity->largeImageKey) {
        json_object_set_new(assets, "large_image", json_string(activity->largeImageKey));
    }
    if (activity->largeImageText) {
        json_object_set_new(assets, "large_text", json_string(activity->largeImageText));
    }
    if (activity->smallImageKey) {
        json_object_set_new(assets, "small_image", json_string(activity->smallImageKey));
    }
    if (activity->smallImageText) {
        json_object_set_new(assets, "small_text", json_string(activity->smallImageText));
    }
    if (activity->partyId) {
        json_object_set_new(party, "id", json_string(activity->partyId));
    }
    if (activity->partySize || activity->partyMax) {
        json_array_append_new(party_size, json_integer(activity->partySize));
        json_array_append_new(party_size, json_integer(activity->partyMax));
        json_object_set_new(party, "size", party_size);
    }
    if (activity->partyPrivacy) {
        json_object_set_new(party, "privacy", json_integer(activity->partyPrivacy));
    }
    if (activity->matchSecret) {
        json_object_set_new(secrets, "match", json_string(activity->matchSecret));
    }
    if (activity->joinSecret) {
        json_object_set_new(secrets, "join", json_string(activity->joinSecret));
    }
    if (activity->spectateSecret) {
        json_object_set_new(secrets, "spectate", json_string(activity->spectateSecret));
    }
    json_object_set_new(activity_json, "assets", assets);
    json_object_set_new(activity_json, "party", party);
    json_object_set_new(activity_json, "secrets", secrets);
    json_object_set_new(activity_json, "timestamps", timestamps);
    json_object_set_new(args, "activity", activity_json);
    json_object_set_new(args, "instance", activity->instance ? json_true() : json_false());
    json_object_set_new(json, "args", args);
    json_object_set_new(json, "nonce", json_integer(self->socket.nonce));
    self->socket.nonce++;
    char *json_data = json_dumps(json, JSON_COMPACT);
    if (!json_data) {
        self->last_error = "JSON data creation failed.";
        return;
    }
    json_decref(json);
    size_t json_length = strlen(json_data);
    if (json_length > MaxRpcFrameSize - sizeof(MessageFrameHeader)) {
        self->last_error = "JSON data is too large.";
        free(json_data);
        return;
    }

    MessageFrame send_frame;
    send_frame.header.opcode = Frame;
    send_frame.header.length = (uint32_t)strlen(json_data);
    memcpy(send_frame.message, json_data, send_frame.header.length);

    if (!Queue_enqueue(&self->queue, &send_frame)) {
        self->last_error = "Queue enqueue failed.";
        free(json_data);
        return;
    }
}

void DiscordRPC_sendInviteResponse(DiscordRPC* self, const char* user_id, char* response) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
        return;
    }

    json_t *json = json_object();
    json_t *args = json_object();
    if (!json) {
        self->last_error = "JSON object creation failed.";
        return;
    }
    json_object_set_new(json, "cmd", json_string(response));
    json_object_set_new(args, "user_id", json_string(user_id));
    json_object_set_new(json, "nonce", json_integer(self->socket.nonce));
    self->socket.nonce++;
    json_object_set_new(json, "args", args);
    char *json_data = json_dumps(json, JSON_COMPACT);
    if (!json_data) {
        self->last_error = "JSON data creation failed.";
        return;
    }
    json_decref(json);
    size_t json_length = strlen(json_data);
    if (json_length > MaxRpcFrameSize - sizeof(MessageFrameHeader)) {
        self->last_error = "JSON data is too large.";
        free(json_data);
        return;
    }

    MessageFrame send_frame;
    send_frame.header.opcode = Frame;
    send_frame.header.length = (uint32_t)strlen(json_data);
    memcpy(send_frame.message, json_data, send_frame.header.length);

    if (!Queue_enqueue(&self->queue, &send_frame)) {
        self->last_error = "Queue enqueue failed.";
        free(json_data);
        return;
    }

    free(json_data);
}

void DiscordRPC_acceptInvite(DiscordRPC* self, const char* user_id) {
    DiscordRPC_sendInviteResponse(self, user_id, "SEND_ACTIVITY_JOIN_INVITE");
}

void DiscordRPC_declineInvite(DiscordRPC* self, const char* user_id) {
    DiscordRPC_sendInviteResponse(self, user_id, "CLOSE_ACTIVITY_JOIN_REQUEST");
}

void DiscordRPC_onDisconnect(SocketConnection* socket, void* data) {
    (void)socket;
    DiscordRPC* self = (DiscordRPC*)data;
    if (self == NULL) {
        return;
    }
    self->state = Disconnected;
    if (self->handlers.disconnected) {
        self->handlers.disconnected(self->connected);
    }
    self->connected = false;
}

bool DiscordRPC_openSocket(DiscordRPC* self) {
    const char* temp_path = DiscordRPC_getTempPath();
    if (temp_path) {
        for (size_t pipe_index = 0; pipe_index < 10; ++pipe_index) {
            char ipc_path[256];
            snprintf(ipc_path, sizeof(ipc_path), "%s/discord-ipc-%zu", temp_path, pipe_index);
            if (SocketConnection_init(&self->socket, ipc_path, DiscordRPC_onDisconnect, self)) {
                self->connected = true;
                self->state = Connected;
                DiscordRPC_initHandlers(self);
                pthread_create(&self->read_thread, NULL, DiscordRPC_readThread, self);
                DiscordRPC_sendHandshake(self);
                pthread_create(&self->message_thread, NULL, DiscordRPC_messageProcessor, self);
                return true;
            }

            self->last_error = "Socket connection failed.";
        }

        return false;
    } else {
        self->last_error = "Temporary path not found.";
    }
    return false;
}

void DiscordRPC_registerEvent(DiscordRPC* self, const char* event) {
    json_t *json = json_object();
    MessageFrame frame;
    frame.header.opcode = Frame;
    if (!json) {
        self->last_error = "JSON object creation failed.";
        return;
    }
    json_object_set_new(json, "cmd", json_string("SUBSCRIBE"));
    json_object_set_new(json, "evt", json_string(event));
    json_object_set_new(json, "nonce", json_integer(self->socket.nonce));
    self->socket.nonce++;
    char *json_data = json_dumps(json, JSON_COMPACT);
    if (!json_data) {
        self->last_error = "JSON data creation failed.";
        json_decref(json);
        return;
    }
    json_decref(json);
    size_t json_length = strlen(json_data);
    if (json_length > MaxRpcFrameSize - sizeof(MessageFrameHeader)) {
        self->last_error = "JSON data is too large.";
        free(json_data);
        return;
    }
    frame.header.length = (uint32_t)strlen(json_data);
    memcpy(frame.message, json_data, frame.header.length);
    if (!Queue_enqueue(&self->queue, &frame)) {
        self->last_error = "Queue enqueue failed.";
        free(json_data);
        return;
    }
    free(json_data);
}

void DiscordRPC_initHandlers(DiscordRPC* self) {
    if (self == NULL) {
        self->last_error = "DiscordRPC instance is NULL.";
        return;
    }
    if (self->handlers.joinRequest || self->handlers.joinGame || self->handlers.spectateGame) {
        if (self->handlers.joinRequest) {
            DiscordRPC_registerEvent(self, "ACTIVITY_JOIN_REQUEST");
        }
        if (self->handlers.joinGame) {
            DiscordRPC_registerEvent(self, "ACTIVITY_JOIN");
        }
        if (self->handlers.spectateGame) {
            DiscordRPC_registerEvent(self, "ACTIVITY_SPECTATE");
        }
    }
}

bool DiscordRPC_sendHandshake(DiscordRPC* self) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
        return false;
    }

    json_t *json = json_object();
    if (!json) {
        self->last_error = "JSON object creation failed.";
        return false;
    }
    json_object_set_new(json, "v", json_integer(1));
    json_object_set_new(json, "client_id", json_string(self->client_id));
    char *json_data = json_dumps(json, JSON_COMPACT);
    if (!json_data) {
        self->last_error = "JSON data creation failed.";
        json_decref(json);
        return false;
    }
    json_decref(json);
    MessageFrame send_frame;
    send_frame.header.opcode = Handshake;
    send_frame.header.length = (uint32_t)strlen(json_data);
    memcpy(send_frame.message, json_data, send_frame.header.length);

    bool success = DiscordRPC_writeMessage(self, &send_frame);
    if (!success) {
        self->last_error = "Handshake message sending failed.";
        return false;
    }

    return true;
}

bool DiscordRPC_read(DiscordRPC* self, void* buffer, size_t size) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
        return false;
    }

    size_t bytes_read = 0;
    while (bytes_read < size) {
        size_t chunkSize = size - bytes_read;
        bool success = SocketConnection_read(&self->socket, (char*)buffer + bytes_read, chunkSize);

        if (!success) {
            self->last_error = "Data reading failed.";
            DiscordRPC_shutdown(self);
            return false;
        }

        bytes_read += chunkSize;
    }

    return true;
}

bool DiscordRPC_readMessage(DiscordRPC* self, MessageFrame* frame) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
        return false;
    }

    bool success = DiscordRPC_read(self, &frame->header, sizeof(MessageFrameHeader));
    if (!success) {
        self->last_error = "Message header reading failed.";
        return false;
    }

    size_t message_size = frame->header.length;
    if (message_size > (MaxRpcFrameSize - sizeof(MessageFrameHeader))) {
        free(frame->message);
        self->last_error = "Message size exceeds maximum limit.";
        return false;
    }

    if (message_size > sizeof(frame->message)) {
        self->last_error = "Message size exceeds maximum buffer size.";
        return false;
    }

    success = DiscordRPC_read(self, frame->message, message_size);
    if (!success) {
        free(frame->message);
        self->last_error = "Message data reading failed.";
        return false;
    }

    return true;
}

bool DiscordRPC_write(DiscordRPC* self, void* data, size_t size) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
        return false;
    }

    return SocketConnection_write(&self->socket, data, size);
}

bool DiscordRPC_writeMessage(DiscordRPC* self, MessageFrame* frame) {
    if (!self->connected) {
        self->last_error = "Connection is not established.";
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
        self->last_error = "Message writing failed.";
        return false;
    }

    return true;
}

void DiscordRPC_shutdown(DiscordRPC* self) {
    if (self->connected) {
        self->connected = false;
        SocketConnection_shutdown(&self->socket);
        pthread_join(self->read_thread, NULL);
        pthread_join(self->message_thread, NULL);
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
            self->last_error = "Message reading failed.";
            continue;
        }
        frame.message[frame.header.length] = '\0'; // null-terminate the message
        // parse message as json
        json_error_t error;
        json_t* json = json_loads(frame.message, 0, &error);
        if (!json) {
            self->last_error = "JSON parsing failed.";
            continue;
        }
        if (!json_is_object(json)) {
            self->last_error = "JSON is not an object.";
            json_decref(json);
            continue;
        }
        // get event string
        const char* event = json_string_value(json_object_get(json, "evt"));
        const char* cmd = json_string_value(json_object_get(json, "cmd"));
        json_t* data = json_object_get(json, "data");
        if (!cmd) {
            self->last_error = "Cmd is not a string.";
            json_decref(json);
            continue;
        }
        // check if event is ready
        if (event && strcmp(event, "READY") == 0 && strcmp(cmd, "DISPATCH") == 0) {
            self->state = SentHandshake;
            // call ready event handler
            if (self->handlers.ready) {
                DiscordUser user;
                json_t* user_json = json_object_get(data, "user");
                if (user_json) {
                    user.id = json_string_value(json_object_get(user_json, "id"));
                    user.username = json_string_value(json_object_get(user_json, "username"));
                    // ignore deprecated warning
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    user.discriminator = json_string_value(json_object_get(user_json, "discriminator"));
                    #pragma GCC diagnostic pop
                    user.global_name = json_string_value(json_object_get(user_json, "global_name"));
                    user.avatar = json_string_value(json_object_get(user_json, "avatar"));
                    user.bot = json_boolean_value(json_object_get(user_json, "bot"));
                    user.flags = json_integer_value(json_object_get(user_json, "flags"));
                    user.premium_type = json_integer_value(json_object_get(user_json, "premium_type"));
                    self->handlers.ready(&user);
                    self->user = user;
                }
            }
        }
        if (event && strcmp(event, "ACTIVITY_JOIN") == 0 && strcmp(cmd, "DISPATCH") == 0) {
            // call join game event handler
            if (self->handlers.joinGame) {
                const char* join_secret = json_string_value(json_object_get(data, "secret"));
                self->handlers.joinGame(join_secret);
            }
        }
        if (event && strcmp(event, "ACTIVITY_SPECTATE") == 0 && strcmp(cmd, "DISPATCH") == 0) {
            // call spectate game event handler
            if (self->handlers.spectateGame) {
                const char* spectate_secret = json_string_value(json_object_get(data, "secret"));
                self->handlers.spectateGame(spectate_secret);
            }
        }
        if (event && strcmp(event, "ACTIVITY_JOIN_REQUEST") == 0 && strcmp(cmd, "DISPATCH") == 0) {
            // call join request event handler
            if (self->handlers.joinRequest) {
                DiscordUser user;
                json_t* user_json = json_object_get(data, "user");
                if (user_json) {
                    user.id = json_string_value(json_object_get(user_json, "id"));
                    user.username = json_string_value(json_object_get(user_json, "username"));
                    // ignore deprecated warning
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    user.discriminator = json_string_value(json_object_get(user_json, "discriminator"));
                    #pragma GCC diagnostic pop
                    user.global_name = json_string_value(json_object_get(user_json, "global_name"));
                    user.avatar = json_string_value(json_object_get(user_json, "avatar"));
                    user.bot = json_boolean_value(json_object_get(user_json, "bot"));
                    user.flags = json_integer_value(json_object_get(user_json, "flags"));
                    user.premium_type = json_integer_value(json_object_get(user_json, "premium_type"));
                    self->handlers.joinRequest(&user);
                }
            }
        }
        if (event && strcmp(event, "ERROR") == 0) {
            // call error event handler
            if (self->handlers.error) {
                const char* error_message = json_string_value(json_object_get(data, "message"));
                int error_code = json_integer_value(json_object_get(data, "code"));
                self->handlers.error(error_code, error_message);
            }
        }
        if (frame.header.opcode == Ping) {
            MessageFrame pong_frame;
            pong_frame.header.opcode = Pong;
            pong_frame.header.length = 0;
            if (!DiscordRPC_writeMessage(self, &pong_frame)) {
                self->last_error = "Pong message writing failed.";
            }
        }
        json_decref(json);
    }

    return NULL;
}

void* DiscordRPC_messageProcessor(void* arg) {
    DiscordRPC* self = (DiscordRPC*)arg;
    MessageFrame frame;

    while (self->connected) {
        if (Queue_isEmpty(&self->queue) || self->state != SentHandshake) {
            // wait for 50ms
            usleep(50000);
            continue;
        }

        Queue_dequeue(&self->queue, &frame);

        if (frame.header.length > 0) { 
            if (!DiscordRPC_writeMessage(self, &frame)) {
                self->last_error = "Message writing failed.";
            }
        }
    }
    return NULL;
}
