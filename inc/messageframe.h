#include <stdint.h>

#define MaxRpcFrameSize 4096

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Handshake = 0,
    Frame = 1,
    Close = 2,
    Ping = 3,
    Pong = 4
} Opcode;

typedef struct {
    Opcode opcode;
    uint32_t length;
} MessageFrameHeader;

typedef struct {
    MessageFrameHeader header;
    char message[MaxRpcFrameSize - sizeof(MessageFrameHeader)];
} MessageFrame;

#ifdef __cplusplus
}
#endif

