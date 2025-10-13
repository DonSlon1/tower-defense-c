//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_GAME_OBJECT_H
#define PROJEKT_GAME_OBJECT_H

#include <raylib.h>
#include <stdbool.h>

typedef enum {
    TOWER,
    ENEMY,
    ANIMATION,
    PROJECTILE
} OBJECT_TYPE;

typedef enum {
    LEVEL_0,
    LEVEL_1,
    LEVEL_MAX
} TOWER_LEVEL;

typedef enum {
    ENEMY_TYPE_SCOUT,
    ENEMY_TYPE_NORMAL,
    ENEMY_TYPE_TANK,
    ENEMY_TYPE_SPEEDY,
    ENEMY_TYPE_COUNT
} ENEMY_TYPE;

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
    // this is in milliseconds
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
    ENEMY_TYPE type;
} ENEMY_DATA;

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
    } data;

} GAME_OBJECT;

#endif //PROJEKT_GAME_OBJECT_H
