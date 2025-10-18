# Network Message System - Usage Guide

## File Structure

```
sources/
├── network.h          # Network API and message definitions
├── network.c          # Network implementation
├── game.c             # Your game logic (where you'll use network)
├── main.c             # Main loop (integration point)
└── ...
```

## How to Send/Receive Messages

### 1. Basic Message Flow

```c
// In your game loop or event handler:

// CREATE a message
network_message msg = network_create_message(
    MSG_TOWER_BUILD,           // Message type
    &tower_data,               // Pointer to data
    sizeof(tower_data)         // Size of data
);

// SEND the message
if (network_send(net, &msg)) {
    printf("Message sent successfully!\n");
} else {
    printf("Failed to send message\n");
}

// RECEIVE messages (call every frame)
network_message received_msg;
while (network_receive(net, &received_msg)) {
    // Process the message
    switch (received_msg.type) {
        case MSG_TOWER_BUILD:
            // Handle tower building
            break;
        case MSG_SEND_ENEMIES:
            // Handle enemy sending
            break;
        // ... other cases
    }
}
```

### 2. Example: Send Tower Build Event

```c
// When player builds a tower
typedef struct {
    uint8_t spot_index;
    vector2 position;
} tower_build_data;

void on_tower_built(game* game, network_state* net, int spot_index) {
    // Build locally
    try_build_tower(game, spot_index);

    // Send to opponent
    tower_build_data data = {
        .spot_index = spot_index,
        .position = game->tower_spots[spot_index].position
    };

    network_message msg = network_create_message(
        MSG_TOWER_BUILD,
        &data,
        sizeof(data)
    );

    network_send(net, &msg);
}
```

### 3. Example: Send Enemies to Opponent

```c
typedef struct {
    uint8_t enemy_type;    // ENEMY_TYPE_MUSHROOM, ENEMY_TYPE_FLYING, etc.
    uint8_t count;         // How many
} send_enemies_data;

void send_enemies_to_opponent(network_state* net, enemy_type type, int count) {
    send_enemies_data data = {
        .enemy_type = type,
        .count = count
    };

    network_message msg = network_create_message(
        MSG_SEND_ENEMIES,
        &data,
        sizeof(data)
    );

    if (network_send(net, &msg)) {
        printf("Sent %d enemies of type %d to opponent!\n", count, type);
    }
}
```

### 4. Example: Receive and Process Messages

```c
// In your main game loop:
void update_multiplayer(game* local_game, game* opponent_game, network_state* net) {
    network_message msg;

    // Process all pending messages
    while (network_receive(net, &msg)) {
        printf("Received message type %d at time %u\n", msg.type, msg.timestamp);

        switch (msg.type) {
            case MSG_TOWER_BUILD: {
                tower_build_data* data = (tower_build_data*)msg.data;

                // Build tower in opponent's game (visible to us)
                try_build_tower(opponent_game, data->spot_index);
                printf("Opponent built tower at spot %d\n", data->spot_index);
                break;
            }

            case MSG_SEND_ENEMIES: {
                send_enemies_data* data = (send_enemies_data*)msg.data;

                // Spawn enemies in OUR game (opponent sent them to attack us!)
                for (int i = 0; i < data->count; i++) {
                    spawn_specific_enemy(local_game, data->enemy_type);
                }
                printf("Opponent sent us %d enemies!\n", data->count);
                break;
            }

            case MSG_PING: {
                // Handle ping/heartbeat
                printf("Ping from opponent\n");
                break;
            }

            case MSG_DISCONNECT: {
                printf("Opponent disconnected!\n");
                // Handle disconnection
                break;
            }
        }
    }
}
```

## Where to Add in Your Code

### In `sources/game.c`:

Add network message sending when actions happen:

```c
// When building tower
bool try_build_tower(game *game, int spot_index) {
    if (game == nullptr) return false;

    // ... existing build logic ...

    // ADD THIS: If multiplayer, send network message
    if (game->network) {  // You'll need to add network_state* to game struct
        tower_build_data data = { .spot_index = spot_index, /* ... */ };
        network_message msg = network_create_message(MSG_TOWER_BUILD, &data, sizeof(data));
        network_send(game->network, &msg);
    }

    return true;
}
```

### In `sources/main.c`:

Add message receiving in the main loop:

```c
// In multiplayer game loop
while (!window_should_close()) {
    // Update local game
    update_game_state(&local_game, delta);

    // Process network messages
    network_message msg;
    while (network_receive(net, &msg)) {
        handle_network_message(&opponent_game, &local_game, &msg);
    }

    // Render both games
    render_multiplayer(&local_game, &opponent_game);
}
```

## Message Types Reference

| Message Type | Purpose | Data Structure |
|---|---|---|
| `MSG_PING` | Heartbeat/keep-alive | Empty |
| `MSG_TOWER_BUILD` | Player built tower | `{ spot_index, position }` |
| `MSG_TOWER_UPGRADE` | Player upgraded tower | `{ tower_id, new_level }` |
| `MSG_SEND_ENEMIES` | Send enemies to opponent | `{ enemy_type, count }` |
| `MSG_GAME_SYNC` | Sync full game state | Game state data |
| `MSG_DISCONNECT` | Player leaving | Empty |

## Performance Tips

1. **Don't flood the network**: Only send messages when actions happen, not every frame
2. **Batch updates**: If possible, combine multiple updates into one message
3. **Use non-blocking receive**: `network_receive()` is non-blocking, safe to call every frame
4. **Check connection**: Use `network_is_connected()` before sending

## Testing Messages

```bash
# Terminal 1 - Host
./build/projekt
# Host game, then send a test message

# Terminal 2 - Client
./build/projekt
# Connect, should receive the test message
```

Check console output for debug messages showing sent/received data!

## Next Steps

1. Add `network_state* network` to your `game` struct
2. When building towers → send `MSG_TOWER_BUILD`
3. When sending enemies → send `MSG_SEND_ENEMIES`
4. In main loop → call `network_receive()` and handle messages
5. Update opponent's game state based on received messages

The hard part (networking) is done! Now just wire up your game events to send messages! 🎮
