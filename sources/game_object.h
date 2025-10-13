#ifndef PROJEKT_GAME_OBJECT_H
#define PROJEKT_GAME_OBJECT_H

#include <raylib.h>

typedef enum {
    TOWER,
    ENEMY,
    PROJECTILE
} OBJECT_TYPE;

typedef enum {
    LEVEL_0,
    LEVEL_1,
    LEVEL_MAX
} TOWER_LEVEL;

typedef enum {
    ENEMY_TYPE_MUSHROOM,
    ENEMY_TYPE_FLYING,
    ENEMY_TYPE_COUNT
} ENEMY_TYPE;

typedef enum {
    ENEMY_ANIM_RUN,
    ENEMY_ANIM_HIT,
    ENEMY_ANIM_DIE
} ENEMY_ANIMATION_STATE;

typedef enum {
    UPGRADE_SUCCESS,
    UPGRADE_INSUFFICIENT_FUNDS,
    UPGRADE_MAX_LEVEL,
    UPGRADE_NOT_FOUND
} UPGRADE_RESULT;

typedef struct {
    const int* sprites;
    int count;
    int width;
    int height;
} SPRITE_INFO;

typedef struct {
    int x;
    int y;
} GRID_COORD;

typedef struct {
    float damage;
    float range;
    float fire_cooldown;
    int target_id;
    int width;
    int height;
    int upgrade_cost;
    TOWER_LEVEL level;
} TOWER_DATA;

typedef struct {
    float health;
    float max_health;
    float speed;
    int waypoint_index;
    int path_id;
    int gold_reward;
    ENEMY_TYPE type;
    ENEMY_ANIMATION_STATE anim_state;
    int current_frame;
    float frame_timer;
} ENEMY_DATA;

typedef struct {
    Vector2 velocity;
    float damage;
    int owner_id;
    int target_id;
    int current_frame;
    float frame_timer;
    int row;
} PROJECTILE_DATA;

typedef struct {
    int current_frame;
    int total_frames;
    float frame_duration;
} ANIMATION_DATA;

typedef struct {
    int id;
    OBJECT_TYPE type;
    Vector2 position;
    bool is_active;

    union {
        TOWER_DATA tower;
        ENEMY_DATA enemy;
        ANIMATION_DATA animation;
        PROJECTILE_DATA projectile;
    } data;

} GAME_OBJECT;

#endif
