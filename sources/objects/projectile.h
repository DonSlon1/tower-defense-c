//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_PROJECTILE_H
#define PROJEKT_PROJECTILE_H

#include "game_object.h"

typedef struct game game;

game_object create_projectile(vector2 start_pos, vector2 target_pos, float damage, int owner_id, int target_id);

void update_projectile(const game *game, game_object *projectile, float delta_time);

#endif //PROJEKT_PROJECTILE_H
