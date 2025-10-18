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

typedef struct game game;

game_object init_tower(vector2 position);

upgrade_result upgrade_clicked_tower(game *game, grid_coord grid_coord);

sprite_info get_tower_sprites(tower_level level);

void update_tower(game *game, game_object *tower, float delta_time);

int find_nearest_enemy_in_range(const game *game, vector2 tower_pos, float range);

#endif //PROJEKT_TOWER_H
