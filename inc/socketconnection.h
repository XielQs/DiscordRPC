#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sock;
    int nonce;
    bool connected;
    char* ipc_path;
} SocketConnection;

bool SocketConnection_init(SocketConnection* conn, const char* ipc_path);
void SocketConnection_shutdown(SocketConnection* conn);
bool SocketConnection_read(SocketConnection* conn, void* buffer, size_t size);
bool SocketConnection_write(SocketConnection* conn, const void* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif // SOCKET_CONNECTION_H

