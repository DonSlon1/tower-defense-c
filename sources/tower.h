//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_TOWER_H
#define PROJEKT_TOWER_H

#include "game_object.h"

// Tower default stats for LEVEL_0
#define TOWER_LEVEL_0_DAMAGE 0.0f
#define TOWER_LEVEL_0_RANGE 0.0f
#define TOWER_LEVEL_0_FIRE_COOLDOWN 1000.0f
#define TOWER_LEVEL_0_WIDTH 4
#define TOWER_LEVEL_0_HEIGHT 4
#define TOWER_LEVEL_0_UPGRADE_COST 100

// Forward declaration
typedef struct GAME GAME;

// Initialize a tower at given position
GAME_OBJECT init_tower(Vector2 position);

// Attempt to upgrade a tower at grid coordinates
UPGRADE_RESULT upgrade_clicked_tower(GAME *game, GRID_COORD grid_coord);

// Get sprite information for tower rendering
SPRITE_INFO get_tower_sprites(TOWER_LEVEL level);

#endif //PROJEKT_TOWER_H
