#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H
#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SocketConnection SocketConnection;
typedef void (*DisconnectCallback)(SocketConnection*, void*);

struct SocketConnection {
    int sock;
    int nonce;
    bool connected;
    char* ipc_path;
    DisconnectCallback on_disconnect;
    void* callback_data;
};

bool SocketConnection_init(SocketConnection* conn, const char* ipc_path, DisconnectCallback on_disconnect,
                            void* callback_data);
void SocketConnection_shutdown(SocketConnection* conn);
bool SocketConnection_read(SocketConnection* conn, void* buffer, size_t size);
bool SocketConnection_write(SocketConnection* conn, const void* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif // SOCKET_CONNECTION_H

