#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

// Protocol version for compatibility checking
#define NETWORK_PROTOCOL_VERSION 1

// Maximum message size
#define MAX_MESSAGE_SIZE 512

// Message types for multiplayer communication
typedef enum {
    msg_ping = 1,              // Heartbeat
    msg_tower_build,           // Player built a tower
    msg_tower_upgrade,         // Player upgraded a tower
    msg_send_enemies,          // Player sends enemies to opponent
    msg_wave_complete,         // Player completed their wave (all enemies dead)
    msg_wave_start,            // Both players ready - start next wave
    msg_game_sync,             // Sync game state
    msg_disconnect,            // Player leaving
    msg_discover_request,      // Broadcast to find sessions
    msg_discover_response,     // Response from a host with session info
} message_type;

// Network message structure
typedef struct {
    uint8_t protocol_version;  // Protocol version (NETWORK_PROTOCOL_VERSION)
    message_type type;         // What kind of message
    uint32_t timestamp;        // When it was sent (SDL_GetTicks())
    uint16_t data_size;        // How much data in payload
    uint8_t data[MAX_MESSAGE_SIZE - 12]; // Payload (512 - 12 bytes overhead = 500 bytes)
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
