# Quick Start Guide - Multiplayer Tower Defense

## ✅ What's Done

1. **Menu System** - Complete UI for single/multiplayer selection
2. **Network Layer** - TCP connections with message sending/receiving
3. **Connection Testing** - Can connect two instances on localhost

## 🎮 How to Use What You Have

### Test Network Connection

**Terminal 1 - Host:**
```bash
./build/projekt
# Click: Multiplayer → Host Game
# Wait for client...
```

**Terminal 2 - Client:**
```bash
./build/projekt
# Click: Multiplayer → Join Game → Connect
```

Both will show "CONNECTION SUCCESSFUL!" ✅

### Send Your First Message

Add this test code to `sources/main.c` after connection succeeds:

```c
// After: menu_set_player_connected(&menu);

// HOST: Send a test message
if (is_host) {
    const char* test_data = "Hello from HOST!";
    network_message msg = network_create_message(
        MSG_PING,
        test_data,
        strlen(test_data) + 1
    );
    network_send(net, &msg);
    printf("HOST: Sent test message\n");
}

// CLIENT: Receive and print message
else {
    network_message msg;
    while (network_receive(net, &msg)) {
        printf("CLIENT: Received message type %d\n", msg.type);
        printf("CLIENT: Data: %s\n", (char*)msg.data);
    }
}
```

## 📁 Where to Add Features

### Want to send tower builds?

**File: `sources/game.c`**
**Function: `try_build_tower()`**

```c
bool try_build_tower(game *game, int spot_index) {
    // ... existing code ...

    // ADD THIS:
    if (game->network) {  // If multiplayer
        typedef struct { uint8_t spot; } tower_msg;
        tower_msg data = { .spot = spot_index };

        network_message msg = network_create_message(
            MSG_TOWER_BUILD,
            &data,
            sizeof(data)
        );
        network_send(game->network, &msg);
    }

    return true;
}
```

### Want to send enemies?

**Create new file: `sources/send_ui.c`**

```c
#include "network.h"
#include "game.h"

void send_enemies_button_clicked(network_state* net, int count) {
    typedef struct { uint8_t type; uint8_t count; } enemy_msg;
    enemy_msg data = { .type = ENEMY_TYPE_MUSHROOM, .count = count };

    network_message msg = network_create_message(
        MSG_SEND_ENEMIES,
        &data,
        sizeof(data)
    );

    if (network_send(net, &msg)) {
        printf("Sent %d enemies to opponent!\n", count);
    }
}
```

### Want to receive messages?

**File: `sources/main.c`**
**In multiplayer game loop:**

```c
// Process network messages every frame
network_message msg;
while (network_receive(net, &msg)) {
    switch (msg.type) {
        case MSG_TOWER_BUILD:
            // Build tower in opponent's game
            break;

        case MSG_SEND_ENEMIES:
            // Spawn enemies in our game
            break;
    }
}
```

## 📚 Documentation Files

- **`MULTIPLAYER_IMPLEMENTATION.md`** - Full implementation plan (all 8 phases)
- **`NETWORK_USAGE.md`** - How to use the network system
- **`PROJECT_STRUCTURE.md`** - File organization guide
- **`test_network.md`** - Testing instructions

## 🎯 Next Immediate Steps

### 1. Make Games Run Simultaneously (Easy!)

Currently, after connection, both show success screen. Change it to run actual games:

**In `sources/main.c`**, replace the success screen with:

```c
if (net) {
    // Start actual game instead of showing success screen
    game local_game = init_game();

    while (!window_should_close()) {
        float delta = get_frame_time();

        // Your game loop here
        update_game_state(&local_game, delta);

        // Check for network messages
        network_message msg;
        while (network_receive(net, &msg)) {
            printf("Received message type %d\n", msg.type);
            // TODO: Handle messages
        }

        // Render
        begin_drawing();
        clear_background(black);
        render_game(&local_game);
        end_drawing();
    }

    unload_game(&local_game);
    network_close(net);
}
```

### 2. Add Two Game States (Medium)

Create separate game states for each player:

```c
game my_game = init_game();        // My game
game opponent_game = init_game();  // What opponent is doing

// When I build tower:
try_build_tower(&my_game, spot);
send_tower_build_message(net, spot);

// When opponent builds tower:
while (network_receive(net, &msg)) {
    if (msg.type == MSG_TOWER_BUILD) {
        try_build_tower(&opponent_game, msg.data.spot);
    }
}
```

### 3. Implement Split-Screen (Advanced)

Render both games side-by-side:

```c
// Left side: my game
render_game_at(&my_game, 0, 0, 800, 600);

// Right side: opponent's game
render_game_at(&opponent_game, 800, 0, 800, 600);
```

## 🔧 Build Commands

```bash
# Reconfigure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run
./build/projekt
```

## 🐛 Debugging Network

Enable debug output in `sources/network.c`:

```c
// In network_send():
printf("[NET] Sending message type %d, size %d bytes\n", msg->type, msg->data_size);

// In network_receive():
printf("[NET] Received message type %d from %s\n",
       out_msg->type, net->is_host ? "CLIENT" : "HOST");
```

## 💡 Pro Tips

1. **Start simple**: Get messages working before adding complex game sync
2. **Test often**: Run two instances after every change
3. **Use printf**: Debug network issues with console output
4. **Check return values**: Always check if `network_send()` returns true
5. **Handle disconnects**: Check `network_is_connected()` regularly

## 🎓 Learning Path

1. ✅ **You are here**: Can establish connections
2. **Next**: Send a simple message
3. **Then**: Send tower build events
4. **After**: Receive and apply messages
5. **Finally**: Full multiplayer with split-screen

## 📞 Quick Reference

```c
// Create connection
network_state* net = network_create_host(7777);        // Host
network_state* net = network_connect("127.0.0.1", 7777); // Client

// Create message
network_message msg = network_create_message(type, &data, size);

// Send message
network_send(net, &msg);

// Receive messages (call every frame)
network_message msg;
while (network_receive(net, &msg)) {
    // Process msg
}

// Cleanup
network_close(net);
```

You're all set! The networking foundation is solid. Now just wire up your game events! 🚀
