#ifndef PROJEKT_GAME_OBJECT_H
#define PROJEKT_GAME_OBJECT_H

#include <raylib.h>

typedef enum {
    tower,
    enemy,
    projectile
} object_type;

typedef enum {
    level_0,
    level_1,
    level_max
} tower_level;

typedef enum {
    enemy_type_mushroom,
    enemy_type_flying,
    enemy_type_count
} enemy_type;

typedef enum {
    enemy_anim_run,
    enemy_anim_hit,
    enemy_anim_die
} enemy_animation_state;

typedef enum {
    upgrade_success,
    upgrade_insufficient_funds,
    upgrade_max_level,
    upgrade_not_found
} upgrade_result;

typedef struct {
    const int* sprites;
    int count;
    int width;
    int height;
} sprite_info;

typedef struct {
    int x;
    int y;
} grid_coord;

typedef struct {
    float damage;
    float range;
    float fire_cooldown;
    int target_id;
    int width;
    int height;
    int upgrade_cost;
    tower_level level;
} tower_data;

typedef struct {
    float health;
    float max_health;
    float speed;
    int waypoint_index;
    int path_id;
    int gold_reward;
    enemy_type type;
    enemy_animation_state anim_state;
    int current_frame;
    float frame_timer;
} enemy_data;

typedef struct {
    vector2 velocity;
    float damage;
    int owner_id;
    int target_id;
    int current_frame;
    float frame_timer;
    int row;
} projectile_data;

typedef struct {
    int current_frame;
    int total_frames;
    float frame_duration;
} animation_data;

typedef struct {
    int id;
    object_type type;
    vector2 position;
    bool is_active;

    union {
        tower_data tower;
        enemy_data enemy;
        animation_data animation;
        projectile_data projectile;
    } data;

} game_object;

#endif
