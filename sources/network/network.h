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
    MSG_WAVE_COMPLETE,         // Player completed their wave (all enemies dead)
    MSG_WAVE_START,            // Both players ready - start next wave
    MSG_GAME_SYNC,             // Sync game state
    MSG_DISCONNECT,            // Player leaving
    MSG_DISCOVER_REQUEST,      // Broadcast to find sessions
    MSG_DISCOVER_RESPONSE,     // Response from a host with session info
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
bool network_host_check_for_client(network_state* net);  // Non-blocking accept check
void network_close(network_state* net);
bool network_is_connected(const network_state* net);

// Message sending/receiving
bool network_send(network_state* net, const network_message* msg);
bool network_receive(network_state* net, network_message* out_msg);

// Helper to create messages
network_message network_create_message(message_type type, const void* data, uint16_t data_size);

// Session discovery
typedef struct {
    char host_name[64];
    char ip_address[16];
    uint16_t port;
} discovered_session;

typedef struct session_discovery session_discovery;

// Create a discovery service for broadcasting/listening for sessions
session_discovery* discovery_create(uint16_t port);
void discovery_close(session_discovery* disc);

// Host: Start listening for discovery requests and auto-respond
void discovery_start_host(session_discovery* disc, const char* host_name, uint16_t game_port);

// Host: Check for and respond to discovery requests (non-blocking, call in main loop)
void discovery_host_update(session_discovery* disc);

// Client: Find available sessions on local network
int discovery_find_sessions(session_discovery* disc, discovered_session* sessions, int max_sessions, float timeout_seconds);

#endif
