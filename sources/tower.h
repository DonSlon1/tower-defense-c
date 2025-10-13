//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_TOWER_H
#define PROJEKT_TOWER_H

#include "game_object.h"

#define TOWER_LEVEL_0_DAMAGE 25.0f
#define TOWER_LEVEL_0_RANGE 5.0f
#define TOWER_LEVEL_0_FIRE_COOLDOWN 1.0f
#define TOWER_LEVEL_0_WIDTH 4
#define TOWER_LEVEL_0_HEIGHT 4
#define TOWER_LEVEL_0_UPGRADE_COST 200

#define TOWER_LEVEL_1_DAMAGE 50.0f
#define TOWER_LEVEL_1_RANGE 8.0f
#define TOWER_LEVEL_1_FIRE_COOLDOWN 0.6f

typedef struct GAME GAME;

GAME_OBJECT init_tower(Vector2 position);

UPGRADE_RESULT upgrade_clicked_tower(GAME *game, GRID_COORD grid_coord);

SPRITE_INFO get_tower_sprites(TOWER_LEVEL level);

void update_tower(GAME *game, GAME_OBJECT *tower, float delta_time);

int find_nearest_enemy_in_range(const GAME *game, Vector2 tower_pos, float range);

#endif //PROJEKT_TOWER_H
