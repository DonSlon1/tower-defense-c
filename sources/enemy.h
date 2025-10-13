//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_ENEMY_H
#define PROJEKT_ENEMY_H

#include "game_object.h"

typedef struct {
    float health;
    float speed;
} ENEMY_STATS;

// Get stats for a specific enemy type
ENEMY_STATS get_enemy_stats(ENEMY_TYPE type);

// Update enemy position and state
void update_enemy(GAME_OBJECT *enemy, float delta_time);

// Get sprite information for enemy rendering
SPRITE_INFO get_enemy_sprites(ENEMY_TYPE type);

#endif //PROJEKT_ENEMY_H
