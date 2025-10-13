# Tower Defense Game

A classic tower defense game written in C23 with SDL2, featuring custom raylib-compatible wrapper for cross-platform compatibility.

![Game Screenshot](docs/screenshot.png)

## Features

- 🎮 Classic tower defense gameplay
- 🗺️ 25x20 tile-based map with two enemy paths
- 👾 Multiple enemy types with animations (Mushroom, Flying)
- 🏰 Upgradeable towers with targeting system
- 🌊 Progressive wave-based difficulty (10 waves)
- 💰 Economy system (money, upgrades, costs)
- 🎯 Homing projectiles with collision detection
- 🖼️ Custom medieval-themed cursor
- 🔊 Fullscreen support with dynamic scaling
- 📊 Wave progress tracking and game statistics

## Screenshots

### Gameplay
The main game features a colorful tilemap with strategic tower placement and enemy waves.

### Game Over Screen
Displays your final stats including wave reached, enemies defeated, and money earned.

## Requirements

### System Dependencies
- **CMake** 3.10 or higher
- **C23 compatible compiler** (GCC 13+ or Clang 16+)
- **SDL2** - Core graphics library
- **SDL2_image** - PNG image loading
- **SDL2_ttf** - TrueType font rendering

### Arch Linux
```bash
sudo pacman -S cmake gcc sdl2 sdl2_image sdl2_ttf
```

### Ubuntu/Debian
```bash
sudo apt install cmake gcc libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Fedora
```bash
sudo dnf install cmake gcc SDL2-devel SDL2_image-devel SDL2_ttf-devel
```

## Building

```bash
# Clone the repository
git clone git@github.com:DonSlon1/tower-defense-c.git
cd tower-defense-c

# Create build directory and compile
cmake -B build -S .
cmake --build build

# Run the game
./build/projekt
```

## How to Play

### Controls
- **Mouse** - Select and place towers
- **Space** - Start game / Restart after game over
- **ESC** - Quit game

### Gameplay
1. **Start**: Press SPACE on the start screen
2. **Build Towers**: Click on grass tiles to build towers ($100 each)
3. **Upgrade Towers**: Click on existing towers to upgrade them
4. **Defend**: Prevent enemies from reaching the end of the path
5. **Survive**: Complete all 10 waves to win!

### Tower System
- **Level 0**: Basic tower - Low damage, short range
- **Level 1**: Upgraded tower - Increased damage and range
- **Cost**: $100 to build, $150 to upgrade

### Economy
- Starting money: $250
- Enemy kill rewards: 10-15 gold per enemy
- Building cost: $100
- Upgrade cost: $150

### Enemy Types
- **Mushroom**: Ground enemy with moderate speed
- **Flying**: Aerial enemy with different movement pattern
- Each wave increases difficulty with more enemies and higher flying chances

## Project Structure

```
projekt/
├── sources/
│   ├── game.c/h          - Core game logic and state management
│   ├── enemy.c/h         - Enemy AI, pathfinding, and animations
│   ├── tower.c/h         - Tower behavior and targeting
│   ├── projectile.c/h    - Projectile physics and collisions
│   ├── renderer.c/h      - All rendering and UI drawing
│   ├── tilemap.c/h       - Map rendering and scaling
│   ├── raylib.c/h        - SDL2-based raylib compatibility layer
│   ├── game_object.h     - Entity definitions and data structures
│   └── main.c            - Entry point
├── assets/
│   ├── images/           - Textures and sprites
│   └── cursor/           - Custom cursor graphics
├── CMakeLists.txt        - Build configuration
└── README.md             - This file
```

## Technical Details

### Custom Raylib Wrapper
This project implements a custom raylib-compatible API using SDL2, allowing the game to run without the original raylib library. Key features include:

- **Texture Management**: PNG loading via SDL2_image
- **Text Rendering**: TrueType fonts via SDL2_ttf
- **Input Handling**: Keyboard and mouse support
- **Frame Timing**: FPS control and delta time
- **Random Number Generation**: Raylib-compatible LCG algorithm

### Rendering System
- Dynamic scaling based on window size
- Supports fullscreen with automatic aspect ratio maintenance
- Tile-based rendering with 16x16 base tiles
- Sprite animations for enemies (run, hit, die states)
- Multi-layer tilemap support

### Game Architecture
- Entity-Component-System inspired design
- Wave-based enemy spawning with configuration
- Targeting system with range and cooldown
- Collision detection for projectiles
- State machine for game flow (start, playing, game over)

## Development

### Code Style
- C23 standard with `constexpr` and modern features
- `nullptr` instead of `NULL`
- Const correctness throughout
- No comments in code (self-documenting)
- Clear error handling and logging

### Memory Management
- Manual memory management with proper cleanup
- Static allocation where possible
- Address sanitizer enabled in debug builds

## Credits

### Assets
- Tileset and sprites: Custom pixel art
- Cursor: Medieval theme from SweezyCursors
- Fonts: DejaVu Sans (system font)

### Libraries
- [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer
- [SDL2_image](https://github.com/libsdl-org/SDL_image) - Image loading
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf) - Font rendering

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### TODO
- [ ] Add sound effects and music
- [ ] More tower types
- [ ] Additional enemy types
- [ ] Power-ups and special abilities
- [ ] Save/load game state
- [ ] Difficulty levels
- [ ] More maps

## Authors

- **Lukáš** - Initial work and development

---

Built with ❤️ using C23 and SDL2
