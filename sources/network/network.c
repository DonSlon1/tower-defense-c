#include "network.h"
#include <SDL2/SDL_net.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct network_state {
    TCPsocket socket;
    SDLNet_SocketSet socket_set;
    bool is_host;
    bool is_connected;
};

network_state* network_create_host(const uint16_t port) {
    // Initialize SDL_net
    if (SDLNet_Init() < 0) {
        fprintf(stderr, "ERROR: SDLNet_Init failed: %s\n", SDLNet_GetError());
        return nullptr;
    }

    network_state* net = calloc(1, sizeof(network_state));
    if (!net) {
        SDLNet_Quit();
        return nullptr;
    }

    // Resolve host address
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, nullptr, port) < 0) {
        fprintf(stderr, "ERROR: SDLNet_ResolveHost failed: %s\n", SDLNet_GetError());
        free(net);
        SDLNet_Quit();
        return nullptr;
    }

    // Open server socket
    // ReSharper disable once CppLocalVariableMayBeConst
    TCPsocket server = SDLNet_TCP_Open(&ip);
    if (!server) {
        const char* error = SDLNet_GetError();
        fprintf(stderr, "ERROR: Failed to bind to port %d: %s\n", port, error);
        fprintf(stderr, "       Port may already be in use or requires elevated privileges.\n");
        free(net);
        SDLNet_Quit();
        return nullptr;
    }


    // Store the server socket in net so we can keep trying to accept
    // We'll accept in a non-blocking way from the main loop
    net->socket = server;  // Store server socket (this is the listening socket)
    net->is_host = true;
    net->is_connected = false;  // Not connected yet
    net->socket_set = nullptr;  // Will create this after accept

    return net;  // Return immediately, connection will be completed later
}

// Non-blocking check for client connection (for host)
bool network_host_check_for_client(network_state* net) {
    if (!net || !net->is_host || net->is_connected) {
        return false;  // Not a host or already connected
    }

    // Try to accept a connection (non-blocking)
    // ReSharper disable once CppLocalVariableMayBeConst
    TCPsocket server_socket = net->socket;
    // ReSharper disable once CppLocalVariableMayBeConst
    TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);

    if (client_socket) {

        // Close the server socket (we don't need it anymore)
        SDLNet_TCP_Close(server_socket);

        // Replace with the client socket
        net->socket = client_socket;
        net->is_connected = true;

        // Create socket set for non-blocking receive
        net->socket_set = SDLNet_AllocSocketSet(1);
        if (!net->socket_set) {
            fprintf(stderr, "ERROR: Failed to allocate socket set\n");
            SDLNet_TCP_Close(net->socket);
            return false;
        }
        SDLNet_TCP_AddSocket(net->socket_set, net->socket);

        return true;  // Connection established!
    }

    return false;  // No client yet
}

network_state* network_connect(const char* host, const uint16_t port) {
    // Initialize SDL_net
    if (SDLNet_Init() < 0) {
        fprintf(stderr, "ERROR: SDLNet_Init failed: %s\n", SDLNet_GetError());
        return nullptr;
    }

    network_state* net = calloc(1, sizeof(network_state));
    if (!net) {
        SDLNet_Quit();
        return nullptr;
    }

    // Resolve host address
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, host, port) < 0) {
        fprintf(stderr, "ERROR: SDLNet_ResolveHost failed for %s:%d - %s\n",
                host, port, SDLNet_GetError());
        free(net);
        SDLNet_Quit();
        return nullptr;
    }


    // Connect to server
    net->socket = SDLNet_TCP_Open(&ip);
    if (!net->socket) {
        fprintf(stderr, "ERROR: SDLNet_TCP_Open failed: %s\n", SDLNet_GetError());
        free(net);
        SDLNet_Quit();
        return nullptr;
    }

    net->is_host = false;
    net->is_connected = true;

    // Create socket set for non-blocking receive
    net->socket_set = SDLNet_AllocSocketSet(1);
    if (!net->socket_set) {
        fprintf(stderr, "ERROR: Failed to allocate socket set\n");
        SDLNet_TCP_Close(net->socket);
        free(net);
        SDLNet_Quit();
        return nullptr;
    }
    SDLNet_TCP_AddSocket(net->socket_set, net->socket);

    return net;
}

void network_close(network_state* net) {
    if (!net) return;

    if (net->socket_set) {
        SDLNet_FreeSocketSet(net->socket_set);
    }

    if (net->socket) {
        SDLNet_TCP_Close(net->socket);
    }

    free(net);
    SDLNet_Quit();
}

bool network_is_connected(const network_state* net) {
    if (net == nullptr) {
        return false;
    }

    return net->is_connected;
}

// Send a message (blocking)
bool network_send(network_state* net, const network_message* msg) {
    if (!net || !net->is_connected || !msg) {
        return false;
    }

    const int sent = SDLNet_TCP_Send(net->socket, msg, sizeof(network_message));
    if (sent < (int)sizeof(network_message)) {
        fprintf(stderr, "ERROR: Failed to send message (sent %d/%zu bytes)\n",
                sent, sizeof(network_message));
        net->is_connected = false;
        return false;
    }

    return true;
}

// Receive a message (non-blocking)
bool network_receive(network_state* net, network_message* out_msg) {
    if (!net || !net->is_connected || !out_msg) {
        return false;
    }

    // Check if data is available (non-blocking check with 0 timeout)
    const int ready = SDLNet_CheckSockets(net->socket_set, 0);
    if (ready <= 0) {
        return false; // No data available
    }

    if (!SDLNet_SocketReady(net->socket)) {
        return false;
    }

    // Receive the message
    const int received = SDLNet_TCP_Recv(net->socket, out_msg, sizeof(network_message));
    if (received <= 0) {
        fprintf(stderr, "ERROR: Connection lost (received %d bytes)\n", received);
        net->is_connected = false;
        return false;
    }

    if (received != sizeof(network_message)) {
        fprintf(stderr, "WARNING: Partial message received (%d/%zu bytes)\n",
                received, sizeof(network_message));
        return false;
    }

    return true;
}

// Helper to create a message
network_message network_create_message(const message_type type, const void* data, const uint16_t data_size) {
    network_message msg = {0};
    msg.type = type;
    msg.timestamp = SDL_GetTicks();
    msg.data_size = data_size;

    if (data && data_size > 0 && data_size <= sizeof(msg.data)) {
        memcpy(msg.data, data, data_size);
    }

    return msg;
}

// Session discovery implementation

#define DISCOVERY_PORT 47777
#define DISCOVERY_MAGIC 0x54445344  // Tower Defense Session Discovery

struct session_discovery {
    UDPsocket socket;
    UDPpacket* packet;
    bool is_host;
    char host_name[64];
    uint16_t game_port;
};

typedef struct {
    uint32_t magic;
    uint8_t message_type;  // 0 = request, 1 = response
    char host_name[64];
    uint16_t game_port;
} __attribute__((packed)) discovery_packet;

session_discovery* discovery_create(const uint16_t port) {
    session_discovery* disc = calloc(1, sizeof(session_discovery));
    if (!disc) return nullptr;

    disc->socket = SDLNet_UDP_Open(port);
    if (!disc->socket) {
        fprintf(stderr, "ERROR: Failed to open UDP socket: %s\n", SDLNet_GetError());
        free(disc);
        return nullptr;
    }

    disc->packet = SDLNet_AllocPacket(sizeof(discovery_packet));
    if (!disc->packet) {
        fprintf(stderr, "ERROR: Failed to allocate UDP packet\n");
        SDLNet_UDP_Close(disc->socket);
        free(disc);
        return nullptr;
    }

    return disc;
}

void discovery_close(session_discovery* disc) {
    if (!disc) return;
    if (disc->packet) SDLNet_FreePacket(disc->packet);
    if (disc->socket) SDLNet_UDP_Close(disc->socket);
    free(disc);
}

void discovery_start_host(session_discovery* disc, const char* host_name, const uint16_t game_port) {
    if (!disc || !host_name) return;

    disc->is_host = true;
    strncpy(disc->host_name, host_name, sizeof(disc->host_name) - 1);
    disc->host_name[sizeof(disc->host_name) - 1] = '\0';
    disc->game_port = game_port;
}

void discovery_host_update(session_discovery* disc) {
    if (!disc || !disc->is_host) return;

    // Check for discovery requests (non-blocking)
    if (SDLNet_UDP_Recv(disc->socket, disc->packet) > 0) {
        if (disc->packet->len == sizeof(discovery_packet)) {
            const auto request = (const discovery_packet*)disc->packet->data;

            // Check if this is a valid discovery request
            if (request->magic == DISCOVERY_MAGIC && request->message_type == 0) {
                // Send response back to requester
                discovery_packet response = {
                    .magic = DISCOVERY_MAGIC,
                    .message_type = 1,
                    .game_port = disc->game_port
                };

                strncpy(response.host_name, disc->host_name, sizeof(response.host_name) - 1);
                response.host_name[sizeof(response.host_name) - 1] = '\0';

                // Send directly to requester
                disc->packet->len = sizeof(discovery_packet);
                memcpy(disc->packet->data, &response, sizeof(discovery_packet));
                SDLNet_UDP_Send(disc->socket, -1, disc->packet);
            }
        }
    }
}

int discovery_find_sessions(session_discovery* disc, discovered_session* sessions, const int max_sessions, const float timeout_seconds) {
    if (!disc || !sessions || max_sessions <= 0) return 0;

    // Send broadcast request
    const discovery_packet request = {
        .magic = DISCOVERY_MAGIC,
        .message_type = 0,  // Request
        .host_name = "",
        .game_port = 0
    };

    IPaddress broadcast_addr;
    SDLNet_ResolveHost(&broadcast_addr, "255.255.255.255", DISCOVERY_PORT);

    disc->packet->address = broadcast_addr;
    disc->packet->len = sizeof(discovery_packet);
    memcpy(disc->packet->data, &request, sizeof(discovery_packet));

    SDLNet_UDP_Send(disc->socket, -1, disc->packet);

    // Listen for responses
    const Uint32 start_time = SDL_GetTicks();
    const Uint32 timeout_ms = (Uint32)(timeout_seconds * 1000.0f);
    int session_count = 0;

    while (SDL_GetTicks() - start_time < timeout_ms && session_count < max_sessions) {
        if (SDLNet_UDP_Recv(disc->socket, disc->packet) > 0) {
            if (disc->packet->len == sizeof(discovery_packet)) {
                const discovery_packet* response = (discovery_packet*)disc->packet->data;

                // Verify magic number and response type
                if (response->magic == DISCOVERY_MAGIC && response->message_type == 1) {
                    // Convert IP to string
                    const Uint32 ip = SDL_SwapBE32(disc->packet->address.host);
                    snprintf(sessions[session_count].ip_address,
                            sizeof(sessions[session_count].ip_address),
                            "%u.%u.%u.%u",
                            ip >> 24 & 0xFF,
                            ip >> 16 & 0xFF,
                            ip >> 8 & 0xFF,
                            ip & 0xFF);

                    strncpy(sessions[session_count].host_name, response->host_name,
                           sizeof(sessions[session_count].host_name) - 1);
                    sessions[session_count].host_name[sizeof(sessions[session_count].host_name) - 1] = '\0';
                    sessions[session_count].port = response->game_port;

                    session_count++;
                }
            }
        }

        SDL_Delay(10);
    }

    return session_count;
}
