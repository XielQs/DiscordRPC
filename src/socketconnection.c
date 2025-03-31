#include "socketconnection.h"
#include <errno.h>

#define MSG_FLAGS 0

bool SocketConnection_init(SocketConnection* conn, const char* ipc_path, DisconnectCallback on_disconnect,
                            void* callback_data) {
    conn->ipc_path = strdup(ipc_path);
    conn->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    conn->nonce = 0;
    conn->connected = false;
    conn->on_disconnect = on_disconnect;
    conn->callback_data = callback_data;

    if (conn->sock < 0) {
        return false;
    }

    int flags = fcntl(conn->sock, F_GETFL, 0);
    fcntl(conn->sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, ipc_path, sizeof(remote.sun_path) - 1);

    if (connect(conn->sock, (struct sockaddr*)&remote, sizeof(remote)) < 0) {
        close(conn->sock);
        return false;
    }

    conn->connected = true;
    return true;
}

void SocketConnection_shutdown(SocketConnection* conn) {
    if (conn->connected) {
        close(conn->sock);
        conn->connected = false;
        if (conn->on_disconnect) {
            conn->on_disconnect(conn, conn->callback_data);
        }
    }
}

bool SocketConnection_read(SocketConnection* conn, void* buffer, size_t size) {
    if (!conn->connected) {
        return false;
    }

    size_t bytes_read = 0;
    while (bytes_read < size) {
        ssize_t res = recv(conn->sock, (char*)buffer + bytes_read, size - bytes_read, MSG_FLAGS);

        if (res < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            SocketConnection_shutdown(conn);
            return false;
        } 
        
        if (res == 0) {
            SocketConnection_shutdown(conn);
            return false;
        }

        bytes_read += res;
    }

    return bytes_read == size;
}

bool SocketConnection_write(SocketConnection* conn, const void* buffer, size_t size) {
    if (!conn->connected) {
        return false;
    }

    size_t bytes_sent = 0;
    while (bytes_sent < size) {
        ssize_t sent_bytes = send(conn->sock, (const char*)buffer + bytes_sent, size - bytes_sent, MSG_FLAGS);
        if (sent_bytes < 0) {
            SocketConnection_shutdown(conn);
            return false;
        }
        bytes_sent += sent_bytes;
    }

    return bytes_sent == size;
}
