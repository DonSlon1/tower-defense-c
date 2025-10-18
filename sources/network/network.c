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
    TCPsocket server = SDLNet_TCP_Open(&ip);
    if (!server) {
        fprintf(stderr, "ERROR: SDLNet_TCP_Open failed: %s\n", SDLNet_GetError());
        free(net);
        SDLNet_Quit();
        return nullptr;
    }

    printf("HOST: Listening on port %d...\n", port);
    printf("HOST: Server socket ready, waiting for connections...\n");

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
    TCPsocket server_socket = net->socket;
    TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);

    if (client_socket) {
        printf("HOST: Client connected!\n");

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

    printf("CLIENT: Connecting to %s:%d...\n", host, port);

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

    printf("CLIENT: Connected successfully!\n");
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
