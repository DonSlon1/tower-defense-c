#ifndef PROJEKT_ENEMY_H
#define PROJEKT_ENEMY_H

#include "game_object.h"

typedef struct {
    float health;
    float speed;
    int gold_reward;
} enemy_stats;

enemy_stats get_enemy_stats(enemy_type type);
void update_enemy(game_object*enemy, float delta_time);
sprite_info get_enemy_sprites(enemy_type type, enemy_animation_state state);
int get_enemy_frame_count(enemy_type type, enemy_animation_state state);
void update_enemy_animation(game_object*enemy, float delta_time);
vector2 get_path_start_position(int path_id);

#endif
