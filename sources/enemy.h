#ifndef PROJEKT_ENEMY_H
#define PROJEKT_ENEMY_H

#include "game_object.h"

typedef struct {
    float health;
    float speed;
    int gold_reward;
} ENEMY_STATS;

ENEMY_STATS get_enemy_stats(ENEMY_TYPE type);
void update_enemy(GAME_OBJECT*enemy, float delta_time);
SPRITE_INFO get_enemy_sprites(ENEMY_TYPE type, ENEMY_ANIMATION_STATE state);
int get_enemy_frame_count(ENEMY_TYPE type, ENEMY_ANIMATION_STATE state);
void update_enemy_animation(GAME_OBJECT*enemy, float delta_time);
Vector2 get_path_start_position(int path_id);

#endif
