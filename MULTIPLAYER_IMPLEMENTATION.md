# Tower Defense - Multiplayer Implementation Plan

## Overview
Converting the tower defense game to support 2-player PvP multiplayer mode where players compete by defending their own maps while sending enemies to each other.

## Game Design Specifications

### Target Features
- ✅ 2 players compete head-to-head
- ✅ Each player has separate game instance (map, towers, lives, money)
- ✅ Split-screen view: Your game (left) + Opponent's game (right)
- ✅ Players can send enemies to opponent (cost money)
- ✅ Show enemy health bars (but hide opponent's gold)
- ✅ Last player standing wins
- ✅ Local testing on single PC via localhost

### UI Layout
```
┌───────────────────────────────┬─────────────────────────────────┐
│     YOUR GAME (Left 800px)    │   OPPONENT VIEW (Right 800px)   │
├───────────────────────────────┼─────────────────────────────────┤
│  Your Map (interactive)       │  Opponent's Map (read-only)     │
│  Lives: ❤❤❤  Gold: $500      │  Opponent Lives: ❤❤             │
│  Wave: 5                      │  Opponent Wave: 5               │
│                               │  Ping: 45ms                     │
│  ┌─────────────────────┐     │                                 │
│  │ SEND ENEMIES:       │     │                                 │
│  │ [Send 5 Weak] $100  │     │                                 │
│  │ [Send 3 Fast] $150  │     │                                 │
│  │ [Send 1 Boss] $300  │     │                                 │
│  └─────────────────────┘     │                                 │
└───────────────────────────────┴─────────────────────────────────┘
```

---

## Implementation Phases

### ✅ Phase 0: Planning (COMPLETED)
- [x] Design game architecture
- [x] Plan UI layout
- [x] Choose networking library (SDL2_net)
- [x] Create implementation roadmap
- [x] Create this tracking document

---

### 🚧 Phase 1: Menu System (IN PROGRESS)

#### Files to Create
- [ ] `sources/menu.h` - Menu system structures and API
- [ ] `sources/menu.c` - Menu rendering and logic

#### Tasks
- [x] Design menu state machine
- [ ] Implement main menu (Single Player / Multiplayer / Quit)
- [ ] Implement multiplayer menu (Host / Join / Back)
- [ ] Implement IP input screen
- [ ] Implement host waiting screen
- [ ] Add keyboard input handling for IP address
- [ ] Add ESC key to return to menu from game
- [ ] Integrate menu into main.c

#### Menu States
```
MENU_STATE_MAIN
  ├─> MENU_STATE_PLAYING_SINGLE (start single player)
  └─> MENU_STATE_MULTIPLAYER_SELECT
        ├─> MENU_STATE_HOST_WAITING (waiting for connection)
        └─> MENU_STATE_JOIN_ENTERING_IP
              └─> MENU_STATE_CONNECTING
                    └─> MENU_STATE_PLAYING_MULTIPLAYER
```

#### Testing Checklist
- [ ] Menu buttons render correctly
- [ ] Mouse hover highlights buttons
- [ ] Click transitions between states
- [ ] IP input accepts numbers and dots
- [ ] Backspace works in IP input
- [ ] Can return to main menu from any screen
- [ ] ESC key returns to menu from game

---

### ⏳ Phase 2: Networking Layer (TODO)

#### Prerequisites
```bash
# Install SDL2_net
sudo pacman -S sdl2_net  # Arch Linux
# OR
sudo apt install libsdl2-net-dev  # Ubuntu
```

#### Files to Create
- [ ] `sources/network.h` - Network API
- [ ] `sources/network.c` - TCP connection handling using SDL_net

#### Update CMakeLists.txt
```cmake
find_package(SDL2_net REQUIRED)
target_link_libraries(projekt SDL2::SDL2_net)
```

#### Core Functions to Implement
```c
network_state* network_create_host(uint16_t port);
network_state* network_connect(const char* host, uint16_t port);
void network_close(network_state* net);
bool network_send(network_state* net, const network_message* msg);
bool network_receive(network_state* net, network_message* out_msg);
bool network_is_connected(const network_state* net);
uint32_t network_get_latency(const network_state* net);
```

#### Tasks
- [ ] Implement network_create_host() - TCP server socket
- [ ] Implement network_connect() - TCP client socket
- [ ] Implement network_send() - Send message via TCP
- [ ] Implement network_receive() - Non-blocking receive
- [ ] Add connection status tracking
- [ ] Add basic error handling
- [ ] Add ping/latency measurement

#### Testing Checklist (Localhost)
- [ ] Host can bind to port 7777
- [ ] Client can connect to 127.0.0.1:7777
- [ ] Connection establishes successfully
- [ ] Can send test messages
- [ ] Can receive messages without blocking
- [ ] Disconnection is detected properly
- [ ] Can run two instances on same PC

#### Testing Method
```bash
# Terminal 1 - Host
./build/projekt
# Select: Multiplayer -> Host Game

# Terminal 2 - Client
./build/projekt
# Select: Multiplayer -> Join Game
# Enter IP: 127.0.0.1
# Click Connect
```

---

### ⏳ Phase 2.5: Game Discovery System (OPTIONAL BUT RECOMMENDED)

This phase adds automatic discovery of active games on the local network, so players don't need to manually enter IP addresses.

#### Overview
Uses **UDP broadcast** to discover active games on the LAN. When a host starts a game, it periodically broadcasts "I'm hosting" messages. Clients listen for these broadcasts and display a list of available games.

#### How It Works
```
HOST (192.168.1.100)              CLIENT (192.168.1.101)
        │                                  │
        │   UDP Broadcast every 2s         │
        ├─────────────────────────────────>│
        │   "TOWER_DEFENSE_HOST"           │
        │   Port: 7777                     │
        │   Player: "Alice"                │
        │                                  │
        │                                  │  Receives broadcast
        │                                  │  Adds to game list
        │                                  │
        │   <User clicks on game>         │
        │                                  │
        │<──── TCP Connect ────────────────┤
        │   (192.168.1.100:7777)          │
        │                                  │
```

#### Files to Create
- [ ] `sources/discovery.h` - Game discovery API
- [ ] `sources/discovery.c` - UDP broadcast implementation

#### discovery.h Structure
```c
#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <stdint.h>
#include <stdbool.h>

#define DISCOVERY_PORT 7778
#define DISCOVERY_MAGIC "TOWER_DEFENSE_v1"
#define MAX_DISCOVERED_GAMES 16

// Discovered game info
typedef struct {
    char host_ip[64];          // IP address to connect to
    uint16_t game_port;        // TCP port for game (7777)
    char host_name[32];        // Player name
    uint32_t last_seen_ms;     // When we last received broadcast
    bool is_active;            // Still broadcasting?
} discovered_game;

// Discovery state
typedef struct discovery_state discovery_state;

// Host functions (broadcasting)
discovery_state* discovery_create_broadcaster(const char* player_name, uint16_t game_port);
void discovery_broadcast(discovery_state* disc);  // Call every 2 seconds
void discovery_stop_broadcasting(discovery_state* disc);

// Client functions (listening)
discovery_state* discovery_create_listener(void);
int discovery_get_games(discovery_state* disc, discovered_game* out_games, int max_games);
void discovery_update(discovery_state* disc);  // Call every frame, removes stale games

// Cleanup
void discovery_close(discovery_state* disc);

#endif
```

#### discovery.c Implementation
```c
#include "discovery.h"
#include <SDL2/SDL_net.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Broadcast message format
typedef struct {
    char magic[32];            // "TOWER_DEFENSE_v1"
    uint16_t game_port;        // 7777
    char host_name[32];        // Player name
    uint32_t timestamp;        // SDL_GetTicks()
} __attribute__((packed)) discovery_message;

struct discovery_state {
    UDPsocket socket;
    UDPpacket* packet;
    bool is_broadcaster;

    // For broadcaster
    char player_name[32];
    uint16_t game_port;
    uint32_t last_broadcast_time;

    // For listener
    discovered_game games[MAX_DISCOVERED_GAMES];
    int game_count;
};

discovery_state* discovery_create_broadcaster(const char* player_name, uint16_t game_port) {
    discovery_state* disc = calloc(1, sizeof(discovery_state));
    if (!disc) return NULL;

    disc->socket = SDLNet_UDP_Open(0);  // Random port for sending
    if (!disc->socket) {
        fprintf(stderr, "ERROR: Failed to open UDP socket for broadcasting\n");
        free(disc);
        return NULL;
    }

    disc->packet = SDLNet_AllocPacket(512);
    if (!disc->packet) {
        SDLNet_UDP_Close(disc->socket);
        free(disc);
        return NULL;
    }

    disc->is_broadcaster = true;
    strncpy(disc->player_name, player_name, 31);
    disc->game_port = game_port;
    disc->last_broadcast_time = 0;

    printf("DISCOVERY: Broadcasting as '%s' on port %d\n", player_name, game_port);
    return disc;
}

void discovery_broadcast(discovery_state* disc) {
    if (!disc || !disc->is_broadcaster) return;

    uint32_t now = SDL_GetTicks();
    if (now - disc->last_broadcast_time < 2000) {
        return;  // Only broadcast every 2 seconds
    }
    disc->last_broadcast_time = now;

    // Create broadcast message
    discovery_message msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.magic, DISCOVERY_MAGIC, 31);
    msg.game_port = disc->game_port;
    strncpy(msg.host_name, disc->player_name, 31);
    msg.timestamp = now;

    // Copy to packet
    memcpy(disc->packet->data, &msg, sizeof(msg));
    disc->packet->len = sizeof(msg);

    // Set broadcast address
    IPaddress addr;
    addr.host = INADDR_BROADCAST;
    addr.port = SDL_SwapBE16(DISCOVERY_PORT);
    disc->packet->address = addr;

    // Send
    if (SDLNet_UDP_Send(disc->socket, -1, disc->packet) == 0) {
        fprintf(stderr, "WARNING: Discovery broadcast failed\n");
    }
}

discovery_state* discovery_create_listener(void) {
    discovery_state* disc = calloc(1, sizeof(discovery_state));
    if (!disc) return NULL;

    disc->socket = SDLNet_UDP_Open(DISCOVERY_PORT);
    if (!disc->socket) {
        fprintf(stderr, "ERROR: Failed to open UDP port %d for discovery\n", DISCOVERY_PORT);
        free(disc);
        return NULL;
    }

    disc->packet = SDLNet_AllocPacket(512);
    if (!disc->packet) {
        SDLNet_UDP_Close(disc->socket);
        free(disc);
        return NULL;
    }

    disc->is_broadcaster = false;
    disc->game_count = 0;

    printf("DISCOVERY: Listening for games on port %d\n", DISCOVERY_PORT);
    return disc;
}

void discovery_update(discovery_state* disc) {
    if (!disc || disc->is_broadcaster) return;

    // Receive all pending broadcasts (non-blocking)
    while (SDLNet_UDP_Recv(disc->socket, disc->packet) > 0) {
        discovery_message msg;
        memcpy(&msg, disc->packet->data, sizeof(msg));

        // Validate magic string
        if (strncmp(msg.magic, DISCOVERY_MAGIC, strlen(DISCOVERY_MAGIC)) != 0) {
            continue;  // Not our protocol
        }

        // Get sender IP
        const char* sender_ip = SDLNet_ResolveIP(&disc->packet->address);
        if (!sender_ip) continue;

        // Check if we already have this game
        int existing_index = -1;
        for (int i = 0; i < disc->game_count; i++) {
            if (strcmp(disc->games[i].host_ip, sender_ip) == 0) {
                existing_index = i;
                break;
            }
        }

        if (existing_index >= 0) {
            // Update existing game
            disc->games[existing_index].last_seen_ms = SDL_GetTicks();
            disc->games[existing_index].is_active = true;
        } else {
            // Add new game
            if (disc->game_count < MAX_DISCOVERED_GAMES) {
                discovered_game* game = &disc->games[disc->game_count];
                strncpy(game->host_ip, sender_ip, 63);
                game->game_port = msg.game_port;
                strncpy(game->host_name, msg.host_name, 31);
                game->last_seen_ms = SDL_GetTicks();
                game->is_active = true;
                disc->game_count++;

                printf("DISCOVERY: Found game hosted by '%s' at %s:%d\n",
                       game->host_name, game->host_ip, game->game_port);
            }
        }
    }

    // Remove stale games (not seen in 5 seconds)
    uint32_t now = SDL_GetTicks();
    for (int i = 0; i < disc->game_count; i++) {
        if (now - disc->games[i].last_seen_ms > 5000) {
            disc->games[i].is_active = false;

            // Shift array down
            for (int j = i; j < disc->game_count - 1; j++) {
                disc->games[j] = disc->games[j + 1];
            }
            disc->game_count--;
            i--;
        }
    }
}

int discovery_get_games(discovery_state* disc, discovered_game* out_games, int max_games) {
    if (!disc || !out_games) return 0;

    int count = disc->game_count < max_games ? disc->game_count : max_games;
    memcpy(out_games, disc->games, count * sizeof(discovered_game));
    return count;
}

void discovery_close(discovery_state* disc) {
    if (!disc) return;

    if (disc->packet) {
        SDLNet_FreePacket(disc->packet);
    }
    if (disc->socket) {
        SDLNet_UDP_Close(disc->socket);
    }
    free(disc);
}
```

#### Update Menu System

Add new menu state for game browser:

```c
// menu.h
typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_MULTIPLAYER_SELECT,
    MENU_STATE_HOST_WAITING,
    MENU_STATE_JOIN_SELECTING_GAME,  // NEW: Browse discovered games
    MENU_STATE_JOIN_ENTERING_IP,
    MENU_STATE_CONNECTING,
    MENU_STATE_PLAYING_SINGLE,
    MENU_STATE_PLAYING_MULTIPLAYER,
    MENU_STATE_QUIT
} menu_state;

// Add to menu_system structure
typedef struct {
    // ... existing fields ...

    discovery_state* discovery;
    discovered_game available_games[MAX_DISCOVERED_GAMES];
    int available_game_count;
    int selected_game_index;
} menu_system;
```

Add game browser UI:

```c
// In update_multiplayer_menu()
if (menu->multi_buttons[1].clicked) {
    // Join game - start discovery
    menu->is_host = false;
    menu->discovery = discovery_create_listener();
    menu->current_state = MENU_STATE_JOIN_SELECTING_GAME;
}

// New update function
static void update_join_browser(menu_system* menu) {
    // Update discovery
    discovery_update(menu->discovery);
    menu->available_game_count = discovery_get_games(
        menu->discovery,
        menu->available_games,
        MAX_DISCOVERED_GAMES
    );

    // Render game list as buttons
    vector2 mouse = GetMousePosition();

    for (int i = 0; i < menu->available_game_count; i++) {
        discovered_game* game = &menu->available_games[i];

        // Create button for each game
        menu_button game_btn = {
            .position = {200, 150 + i * 60},
            .size = {400, 50},
            .text = game->host_name
        };

        update_button(&game_btn);

        if (game_btn.clicked) {
            // Connect to this game
            strncpy(menu->ip_input, game->host_ip, 63);
            menu->port_number = game->game_port;
            menu->current_state = MENU_STATE_CONNECTING;
            discovery_close(menu->discovery);
            menu->discovery = NULL;
        }
    }

    // Manual IP button
    menu_button manual_btn = {
        .position = {200, 450},
        .size = {200, 40},
        .text = "Enter IP Manually"
    };
    update_button(&manual_btn);
    if (manual_btn.clicked) {
        menu->current_state = MENU_STATE_JOIN_ENTERING_IP;
        discovery_close(menu->discovery);
        menu->discovery = NULL;
    }

    // Back button
    menu_button back_btn = {
        .position = {420, 450},
        .size = {100, 40},
        .text = "Back"
    };
    update_button(&back_btn);
    if (back_btn.clicked) {
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;
        discovery_close(menu->discovery);
        menu->discovery = NULL;
    }
}

// Render function
static void render_join_browser(const menu_system* menu) {
    DrawText("AVAILABLE GAMES", 240, 80, 30, WHITE);

    if (menu->available_game_count == 0) {
        DrawText("Searching for games...", 250, 250, 20, GRAY);

        // Animated dots
        int dots = (SDL_GetTicks() / 500) % 4;
        char dot_str[8] = "";
        for (int i = 0; i < dots; i++) strcat(dot_str, ".");
        DrawText(dot_str, 480, 250, 20, GRAY);
    } else {
        // Render game list
        for (int i = 0; i < menu->available_game_count; i++) {
            discovered_game* game = &menu->available_games[i];

            menu_button game_btn = {
                .position = {200, 150 + i * 60},
                .size = {400, 50}
            };

            render_button(&game_btn);

            // Game info
            char info[128];
            snprintf(info, sizeof(info), "%s @ %s:%d",
                    game->host_name, game->host_ip, game->game_port);
            DrawText(info, game_btn.position.x + 10,
                    game_btn.position.y + 15, 18, WHITE);
        }
    }

    DrawText("Or:", 360, 420, 16, GRAY);
    // Render manual IP and back buttons...
}
```

#### Update Host to Broadcast

```c
// In update_host_waiting()
if (!menu->discovery && menu->waiting_for_player) {
    // Start broadcasting
    menu->discovery = discovery_create_broadcaster("Player", 7777);
}

// Call every frame while waiting
if (menu->discovery) {
    discovery_broadcast(menu->discovery);
}

// When player connects, stop broadcasting
if (player_connected) {
    discovery_close(menu->discovery);
    menu->discovery = NULL;
}
```

#### Tasks
- [ ] Implement discovery.h and discovery.c
- [ ] Add UDP broadcast support
- [ ] Create game browser UI in menu
- [ ] Update host to broadcast presence
- [ ] Update client to show discovered games
- [ ] Add "Enter IP Manually" fallback option
- [ ] Handle stale game removal (timeout)
- [ ] Test with multiple hosts on LAN

#### Testing Checklist
- [ ] Host broadcasts every 2 seconds
- [ ] Client receives broadcasts
- [ ] Games appear in browser list
- [ ] Can click on game to connect
- [ ] Stale games removed after 5 seconds
- [ ] Works on same PC (127.0.0.1)
- [ ] Works on LAN (192.168.x.x)
- [ ] Manual IP entry still works

#### Benefits
✅ No need to remember/type IP addresses
✅ Automatic discovery on LAN
✅ Shows available games in real-time
✅ Better user experience
✅ Still allows manual IP for internet play

#### Alternative: mDNS/Bonjour
For more advanced discovery (like Minecraft LAN games):
- Use Avahi (Linux) or Bonjour (cross-platform)
- Automatic service registration
- More complex but more robust
- Requires additional library dependencies

---

### ⏳ Phase 3: Network Protocol (TODO)

#### Files to Create
- [ ] `sources/protocol.h` - Message structures and helpers
- [ ] `sources/protocol.c` - Message packing/unpacking

#### Message Types
```c
typedef enum {
    MSG_HANDSHAKE = 1,      // Initial connection
    MSG_PLAYER_READY,       // Ready to start
    MSG_GAME_START,         // Game begins
    MSG_TOWER_BUILD,        // Player built tower
    MSG_TOWER_UPGRADE,      // Player upgraded tower
    MSG_SEND_ENEMIES,       // Player sends enemies to opponent
    MSG_ENEMY_DAMAGED,      // Enemy took damage
    MSG_ENEMY_DIED,         // Enemy died
    MSG_WAVE_COMPLETE,      // Wave finished
    MSG_PLAYER_DEFEATED,    // Player lost
    MSG_PING,               // Heartbeat
    MSG_DISCONNECT,         // Leaving
} message_type;
```

#### Tasks
- [ ] Define network_message structure
- [ ] Define payload structures for each message type
- [ ] Implement message creation helpers
- [ ] Implement message handling dispatcher
- [ ] Add message serialization (if needed)
- [ ] Add timestamp tracking

#### Testing Checklist
- [ ] Can create all message types
- [ ] Messages pack correctly into byte array
- [ ] Messages unpack correctly
- [ ] Payload data survives serialization
- [ ] Message handler routes to correct functions

---

### ⏳ Phase 4: Game State Synchronization (TODO)

#### Files to Modify
- [ ] `sources/game.h` - Add multiplayer structures
- [ ] `sources/game.c` - Add network event handling

#### New Structures
```c
typedef struct {
    game player_games[2];      // Two separate game instances
    player_info players[2];    // Player info
    uint32_t local_player_id;  // 0 or 1
    uint32_t opponent_id;      // 1 or 0
    void* network;             // Network handle
    bool is_host;
} multiplayer_game;
```

#### Tasks
- [ ] Create multiplayer_game structure
- [ ] Initialize two separate game instances
- [ ] Add spawn_specific_enemy() function
- [ ] Modify handle_playing_input() to send network messages
- [ ] Modify try_build_tower() to broadcast to network
- [ ] Add network message receiver loop
- [ ] Apply network messages to opponent's game state
- [ ] Add win/lose condition detection

#### Events to Synchronize
- [ ] Tower built → Send MSG_TOWER_BUILD
- [ ] Tower upgraded → Send MSG_TOWER_UPGRADE
- [ ] Enemies sent → Send MSG_SEND_ENEMIES
- [ ] Enemy died → Send MSG_ENEMY_DIED (for score sync)
- [ ] Player defeated → Send MSG_PLAYER_DEFEATED

#### Testing Checklist
- [ ] Building tower on client appears on host's opponent view
- [ ] Building tower on host appears on client's opponent view
- [ ] Sending enemies spawns them in opponent's game
- [ ] Both players see correct enemy counts
- [ ] Game states stay synchronized
- [ ] Lag doesn't break synchronization

---

### ⏳ Phase 5: Enemy Sending System (TODO)

#### Files to Create
- [ ] `sources/send_ui.h` - Enemy sending UI
- [ ] `sources/send_ui.c` - Button rendering and logic

#### Tasks
- [ ] Create send_button structure
- [ ] Create send_ui structure
- [ ] Implement send UI rendering (left side of screen)
- [ ] Add button hover/click detection
- [ ] Add money cost checking
- [ ] Send MSG_SEND_ENEMIES on button click
- [ ] Deduct money when sending enemies
- [ ] Add visual feedback on send

#### Enemy Types to Send
```c
[Send 5 Weak Mushrooms] - $50
[Send 3 Fast Flyers]    - $100
[Send 2 Tanky Enemies]  - $150
[Send 1 Boss]           - $300
```

#### Testing Checklist
- [ ] Send buttons render on left side
- [ ] Buttons respond to hover
- [ ] Click sends network message
- [ ] Money is deducted correctly
- [ ] Can't send if not enough money
- [ ] Enemies appear in opponent's game
- [ ] Correct enemy type spawns
- [ ] Correct count spawns

---

### ⏳ Phase 6: Split-Screen Rendering (TODO)

#### Files to Modify
- [ ] `sources/renderer.h` - Add multiplayer rendering
- [ ] `sources/renderer.c` - Implement split view

#### Tasks
- [ ] Increase window size to 1600x600
- [ ] Create render_game_viewport() function
- [ ] Render local game on left (0, 0, 800, 600)
- [ ] Render opponent game on right (800, 0, 800, 600)
- [ ] Draw divider line between views
- [ ] Show opponent's lives and wave (but NOT gold)
- [ ] Show connection status (ping)
- [ ] Add "YOU WIN" / "YOU LOSE" overlay

#### UI Elements
- [ ] Left side: Full UI (money, build buttons, send UI)
- [ ] Right side: Minimal UI (only lives, wave, ping)
- [ ] Divider line at x=800
- [ ] Connection status indicator
- [ ] Win/lose screen

#### Testing Checklist
- [ ] Both viewports render correctly
- [ ] Left viewport is interactive
- [ ] Right viewport is read-only
- [ ] Can't click on opponent's game
- [ ] Opponent's gold is hidden
- [ ] Opponent's health bars are visible
- [ ] Ping displays correctly

---

### ⏳ Phase 7: Main Loop Integration (TODO)

#### Files to Modify
- [ ] `sources/main.c` - Add multiplayer game loop

#### Tasks
- [ ] Detect menu_should_start_multiplayer()
- [ ] Initialize network based on is_host
- [ ] Create multiplayer_game structure
- [ ] Run separate update loops for both games
- [ ] Poll network messages each frame
- [ ] Apply network messages to game state
- [ ] Check for win/lose conditions
- [ ] Handle disconnection gracefully

#### Game Loop Structure
```c
while (!WindowShouldClose()) {
    // Local input
    handle_multiplayer_input(&mp_game);

    // Network sync
    while (network_receive(mp_game.network, &msg)) {
        protocol_handle_message(&mp_game, &msg);
    }

    // Update both games
    update_game_state(&mp_game.player_games[0], dt);
    update_game_state(&mp_game.player_games[1], dt);

    // Render split screen
    render_multiplayer_game(&mp_game);
}
```

#### Testing Checklist
- [ ] Both games update simultaneously
- [ ] Network messages apply correctly
- [ ] Input only affects local game
- [ ] Opponent's actions appear in real-time
- [ ] Frame rate stays stable
- [ ] No race conditions

---

### ⏳ Phase 8: Polish & Testing (TODO)

#### Bug Fixes
- [ ] Fix synchronization bugs
- [ ] Handle network disconnection
- [ ] Add reconnection support (optional)
- [ ] Fix race conditions
- [ ] Prevent cheating (validate moves on both sides)

#### Quality of Life
- [ ] Add sound effects for enemy sends
- [ ] Add chat messages ("Player sent enemies!")
- [ ] Add victory/defeat animations
- [ ] Add statistics screen (enemies sent, money spent, etc.)
- [ ] Add rematch option

#### Performance
- [ ] Profile network bandwidth usage
- [ ] Optimize message frequency
- [ ] Add message batching if needed
- [ ] Test with artificial lag

#### Documentation
- [ ] Update README.md with multiplayer instructions
- [ ] Document network protocol
- [ ] Add troubleshooting section
- [ ] Create video demonstration

---

## Testing Strategy

### Local Testing (One PC)
```bash
# Build
cmake --build build -j$(nproc)

# Terminal 1 - Host
./build/projekt
# Select: Multiplayer -> Host Game

# Terminal 2 - Client
./build/projekt
# Select: Multiplayer -> Join Game
# Enter: 127.0.0.1
# Click: Connect
```

### LAN Testing (Two PCs)
```bash
# On Host PC - Find IP address
ip addr show | grep "inet "
# Example output: inet 192.168.1.100/24

# Host starts game
./build/projekt
# Select: Multiplayer -> Host Game

# On Client PC
./build/projekt
# Select: Multiplayer -> Join Game
# Enter: 192.168.1.100 (host's IP)
# Click: Connect
```

### Debug Output
Add to network.c:
```c
#define NETWORK_DEBUG 1

#if NETWORK_DEBUG
#define NET_LOG(...) printf("[NET] " __VA_ARGS__)
#else
#define NET_LOG(...)
#endif
```

---

## Known Issues / Gotchas

### Potential Problems
- [ ] Port 7777 might be blocked by firewall
- [ ] Clock drift between clients (use timestamps)
- [ ] Network messages might arrive out of order
- [ ] Disconnection during game
- [ ] Host quits and client crashes
- [ ] Different frame rates on different machines

### Solutions
- Use SDL_GetTicks() for timestamps
- Add sequence numbers to messages
- Implement timeout detection
- Add graceful disconnection handling
- Make client robust to host disconnection

---

## Time Estimates

| Phase | Tasks | Estimated Time | Status |
|-------|-------|----------------|--------|
| Phase 0 | Planning | 1 hour | ✅ DONE |
| Phase 1 | Menu System | 4-6 hours | 🚧 IN PROGRESS |
| Phase 2 | Networking Layer | 6-8 hours | ⏳ TODO |
| Phase 3 | Protocol | 4-5 hours | ⏳ TODO |
| Phase 4 | Game Sync | 8-10 hours | ⏳ TODO |
| Phase 5 | Enemy Sending | 4-6 hours | ⏳ TODO |
| Phase 6 | Split Screen | 6-8 hours | ⏳ TODO |
| Phase 7 | Integration | 4-6 hours | ⏳ TODO |
| Phase 8 | Polish & Test | 8-12 hours | ⏳ TODO |
| **TOTAL** | | **45-61 hours** | **~2-3 weeks part-time** |

---

## Progress Tracking

### Current Focus
**Phase 1: Menu System**
- Creating menu.h and menu.c
- Next: Integrate into main.c and test navigation

### Completed
- [x] Overall design
- [x] Implementation plan
- [x] This tracking document

### Next Steps
1. Create menu.h and menu.c files
2. Update main.c to use menu system
3. Build and test menu navigation
4. Move to Phase 2 (Networking)

---

## Resources

### Documentation
- SDL2_net API: https://www.libsdl.org/projects/SDL_net/docs/
- Network programming guide: https://beej.us/guide/bgnet/

### Similar Games
- Bloons TD Battles (reference for PvP tower defense)
- Kingdom Rush (reference for UI layout)

### Tools
- Wireshark - Network traffic analysis
- valgrind - Memory leak detection
- gdb - Debugging network issues

---

## Notes

### Design Decisions
- **Why TCP?** - Reliable delivery, game is not latency-critical
- **Why SDL_net?** - Simple API, integrates with SDL2, cross-platform
- **Why localhost testing?** - Easy development, no network complexity
- **Why split-screen?** - Players can see opponent's strategy

### Alternative Approaches Considered
- **UDP with ENet** - More complex, not needed for 2-player game
- **Single game state** - Harder to sync, more network traffic
- **Turn-based** - Less exciting gameplay

---

*Last Updated: 2025-10-18*
*Version: 1.0*
