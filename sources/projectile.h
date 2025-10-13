//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_PROJECTILE_H
#define PROJEKT_PROJECTILE_H

#include "game_object.h"

typedef struct GAME GAME;

GAME_OBJECT create_projectile(Vector2 start_pos, Vector2 target_pos, float damage, int owner_id, int target_id);

void update_projectile(const GAME *game, GAME_OBJECT *projectile, float delta_time);

#endif //PROJEKT_PROJECTILE_H
