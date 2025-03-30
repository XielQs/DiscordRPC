#define _POSIX_C_SOURCE 200809L
#include "socketconnection.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#define MSG_FLAGS 0

bool SocketConnection_init(SocketConnection* conn, const char* ipc_path) {
    conn->ipc_path = strdup(ipc_path);
    conn->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    conn->nonce = 0;

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
    }
}

bool SocketConnection_read(SocketConnection* conn, void* buffer, size_t size) {
    if (!conn->connected) {
        return false;
    }

    size_t bytesRead = 0;
    while (bytesRead < size) {
        ssize_t res = recv(conn->sock, (char*)buffer + bytesRead, size - bytesRead, MSG_FLAGS);
        
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

        bytesRead += res;
    }

    return bytesRead == size;
}


bool SocketConnection_write(SocketConnection* conn, const void* buffer, size_t size) {
    if (!conn->connected) {
        return false;
    }

    size_t bytesSent = 0;
    while (bytesSent < size) {
        ssize_t sentBytes = send(conn->sock, (const char*)buffer + bytesSent, size - bytesSent, MSG_FLAGS);
        if (sentBytes < 0) {
            SocketConnection_shutdown(conn);
            return false;
        }
        bytesSent += sentBytes;
    }

    return bytesSent == size;
}
