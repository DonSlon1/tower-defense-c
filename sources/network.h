#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stdint.h>

// Maximum message size
#define MAX_MESSAGE_SIZE 512

// Message types for multiplayer communication
typedef enum {
    MSG_PING = 1,              // Heartbeat
    MSG_TOWER_BUILD,           // Player built a tower
    MSG_TOWER_UPGRADE,         // Player upgraded a tower
    MSG_SEND_ENEMIES,          // Player sends enemies to opponent
    MSG_GAME_SYNC,             // Sync game state
    MSG_DISCONNECT,            // Player leaving
} message_type;

// Network message structure
typedef struct {
    message_type type;         // What kind of message
    uint32_t timestamp;        // When it was sent (SDL_GetTicks())
    uint16_t data_size;        // How much data in payload
    uint8_t data[MAX_MESSAGE_SIZE - 8]; // Payload (512 - 8 bytes overhead = 504 bytes)
} __attribute__((packed)) network_message;

// Network state (opaque)
typedef struct network_state network_state;

// Connection management
network_state* network_create_host(uint16_t port);
network_state* network_connect(const char* host, uint16_t port);
void network_close(network_state* net);
bool network_is_connected(const network_state* net);

// Message sending/receiving
bool network_send(network_state* net, const network_message* msg);
bool network_receive(network_state* net, network_message* out_msg);

// Helper to create messages
network_message network_create_message(message_type type, const void* data, uint16_t data_size);

#endif
