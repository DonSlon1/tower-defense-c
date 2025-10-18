# Project Structure

## Current File Organization

```
projekt/
├── assets/                          # Game assets
│   ├── images/                     # Textures and sprites
│   ├── cursor/                     # Custom cursors
│   └── fonts/                      # TTF fonts
│
├── sources/                         # Source code (organized by category)
│   ├── core/                       # Core game systems
│   │   ├── main.c                  # Entry point, main loop
│   │   ├── game.h / game.c         # Game state, logic
│   │   ├── game_object.h           # Game object definitions
│   │   └── renderer.h / renderer.c # Rendering system
│   │
│   ├── objects/                    # Game entity implementations
│   │   ├── enemy.h / enemy.c       # Enemy behavior
│   │   ├── tower.h / tower.c       # Tower behavior
│   │   ├── projectile.h / projectile.c # Projectile behavior
│   │   └── tilemap.h / tilemap.c   # Map/tilemap system
│   │
│   ├── ui/                         # User interface
│   │   └── menu.h / menu.c         # Menu system (Phase 1 ✅)
│   │
│   ├── network/                    # Multiplayer networking
│   │   └── network.h / network.c   # Network layer (Phase 2 ✅)
│   │
│   └── utils/                      # Utility libraries
│       └── raylib.h / raylib.c     # Custom SDL2 raylib wrapper
│
├── build/                           # Build output (generated)
│   └── projekt                     # Executable
│
├── Documentation
│   ├── README.md                   # Project README
│   ├── LICENSE                     # MIT License
│   ├── MULTIPLAYER_IMPLEMENTATION.md # Full implementation guide
│   ├── NETWORK_USAGE.md            # How to use networking (Phase 2 ✅)
│   ├── PROJECT_STRUCTURE.md        # This file
│   ├── QUICK_START.md              # Getting started guide
│   └── test_network.md             # Testing instructions
│
└── CMakeLists.txt                   # Build configuration
```

## File Responsibilities

### Core Game Files

**`main.c`** (130 lines)
- Main entry point
- Window initialization
- Menu loop
- Network connection handling
- Game mode switching (single/multiplayer)

**`game.h` / `game.c`** (90 / 542 lines)
- `game` struct - main game state
- Wave system
- Tower placement
- Enemy spawning
- Game state management
- Input handling

**`game_object.h`**
- Unified game object structure
- Type definitions (tower, enemy, projectile)

**`renderer.h` / `renderer.c`**
- Rendering all game objects
- Drawing UI elements
- Split-screen rendering (TODO: Phase 6)

### Game Object Files

**`enemy.h` / `enemy.c`** (47 / 168 lines)
- Enemy pathfinding
- Animation states
- Health management
- Movement along waypoints

**`tower.h` / `tower.c`** (60 / 175 lines)
- Tower types and levels
- Targeting system
- Firing logic
- Upgrade mechanics

**`projectile.h` / `projectile.c`** (30 / 102 lines)
- Projectile movement
- Homing behavior
- Hit detection
- Damage application

**`tilemap.h` / `tilemap.c`** (30 / 88 lines)
- Tile-based map rendering
- Grid coordinate system
- Tile types (path, grass, etc.)

### UI Files

**`menu.h` / `menu.c`** (72 / 560 lines) ✅ Phase 1 Complete
- Main menu
- Multiplayer menu
- Host/Join screens
- IP input
- Connection status
- Game discovery UI (TODO: Phase 2.5)

### Network Files

**`network.h` / `network.c`** (44 / 217 lines) ✅ Phase 2 Complete
- TCP connection management
- Message sending/receiving
- Protocol definitions
- Non-blocking I/O

**Message Types:**
- `MSG_PING` - Heartbeat
- `MSG_TOWER_BUILD` - Tower placement sync
- `MSG_TOWER_UPGRADE` - Tower upgrade sync
- `MSG_SEND_ENEMIES` - Send enemies to opponent
- `MSG_GAME_SYNC` - Full state sync
- `MSG_DISCONNECT` - Clean disconnect

### Utility Files

**`raylib.h` / `raylib.c`** (75 / 580 lines)
- SDL2 wrapper mimicking raylib API
- Custom cursor support
- Drawing primitives
- Input handling
- Texture loading

## Adding New Features

### Where to Add Multiplayer Features

1. **Enemy Sending UI** → Create `sources/send_ui.h` / `send_ui.c`
2. **Protocol Handlers** → Add to `sources/network.c`
3. **Game Discovery** → Create `sources/discovery.h` / `discovery.c`
4. **Split-screen Rendering** → Modify `sources/renderer.c`

### Where to Add Network Messages

**When building a tower:**
```c
// In sources/game.c, function try_build_tower()
if (multiplayer_mode) {
    network_message msg = network_create_message(MSG_TOWER_BUILD, &data, size);
    network_send(net, &msg);
}
```

**When sending enemies:**
```c
// In sources/send_ui.c (to be created)
void on_send_button_click() {
    network_message msg = network_create_message(MSG_SEND_ENEMIES, &data, size);
    network_send(net, &msg);
}
```

**Receiving messages:**
```c
// In sources/main.c, multiplayer game loop
network_message msg;
while (network_receive(net, &msg)) {
    handle_message(&game, &msg);  // Process the message
}
```

## Build System

**CMakeLists.txt**
- C23 standard
- SDL2, SDL2_image, SDL2_ttf, SDL2_net
- Automatic source file discovery
- Address sanitizer enabled (debug mode)

**Build commands:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/projekt
```

## Dependencies

- **SDL2** - Core graphics/window/input
- **SDL2_image** - PNG/JPG texture loading
- **SDL2_ttf** - Font rendering
- **SDL2_net** - Network communication (TCP)

## Code Statistics

| Category | Files | Lines |
|----------|-------|-------|
| Core Game | 6 | ~1200 |
| Game Objects | 6 | ~600 |
| UI/Menu | 2 | ~630 |
| Network | 2 | ~260 |
| Utilities | 2 | ~650 |
| **Total** | **18** | **~3340** |

## Implementation Status

| Phase | Status | Files |
|-------|--------|-------|
| 0. Planning | ✅ Done | Documentation |
| 1. Menu System | ✅ Done | menu.h/c |
| 2. Networking | ✅ Done | network.h/c |
| 2.5. Game Discovery | ⏳ TODO | discovery.h/c |
| 3. Protocol | ⏳ TODO | Extend network.c |
| 4. Game Sync | ⏳ TODO | Modify game.c |
| 5. Enemy Sending | ⏳ TODO | send_ui.h/c |
| 6. Split-screen | ⏳ TODO | Modify renderer.c |
| 7. Integration | ⏳ TODO | Modify main.c |
| 8. Polish | ⏳ TODO | All files |

## Next Steps

1. **Phase 3**: Add message handling in main.c
2. **Phase 4**: Sync tower building between players
3. **Phase 5**: Create enemy sending UI
4. **Phase 6**: Implement split-screen rendering

See `MULTIPLAYER_IMPLEMENTATION.md` for detailed implementation guide!
