//
// Created by lukas on 10/13/25.
//

#include "enemy.h"
#include <stdio.h>
#include <math.h>

// Path definitions
Vector2 path_0_waypoints[] = {
    (Vector2) {0, 2},
    (Vector2) {19, 2},
    (Vector2) {19, 7},
    (Vector2) {24, 7}
};

Vector2 path_1_waypoints[] = {
    (Vector2) {0, 15},
    (Vector2) {19, 15},
    (Vector2) {19, 10},
    (Vector2) {24, 10}
};

constexpr int path_0_count = sizeof(path_0_waypoints) / sizeof(path_0_waypoints[0]);
constexpr int path_1_count = sizeof(path_1_waypoints) / sizeof(path_1_waypoints[0]);

Vector2* paths[] = { path_0_waypoints, path_1_waypoints };
constexpr int path_counts[] = { path_0_count, path_1_count };

ENEMY_STATS get_enemy_stats(const ENEMY_TYPE type) {
    switch (type) {
        case ENEMY_TYPE_SCOUT:
            return (ENEMY_STATS) { .health = 50.0f, .speed = 3.5f };
        case ENEMY_TYPE_NORMAL:
            return (ENEMY_STATS) { .health = 100.0f, .speed = 2.5f };
        case ENEMY_TYPE_TANK:
            return (ENEMY_STATS) { .health = 200.0f, .speed = 1.5f };
        case ENEMY_TYPE_SPEEDY:
            return (ENEMY_STATS) { .health = 40.0f, .speed = 4.5f };
        default:
            return (ENEMY_STATS) { .health = 100.0f, .speed = 2.5f };
    }
}

void update_enemy(GAME_OBJECT *enemy, float delta_time) {
    if (!enemy->is_active) {
        return;
    }

    const int path_id = enemy->data.enemy.path_id;
    const int waypoint_count = path_counts[path_id];
    const Vector2* current_path = paths[path_id];

    if (enemy->data.enemy.waypoint_index >= waypoint_count) {
        enemy->is_active = false;
        return;
    }

    const Vector2 target_pos = current_path[enemy->data.enemy.waypoint_index];

    const Vector2 direction = {
        target_pos.x - enemy->position.x,
        target_pos.y - enemy->position.y
    };

    const float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);

    if (distance < 0.01f) {
        enemy->position = target_pos;
        enemy->data.enemy.waypoint_index++;
    } else {
        const float abs_dx = fabsf(direction.x);
        const float abs_dy = fabsf(direction.y);

        const float distance_to_move = enemy->data.enemy.speed * delta_time;

        if (abs_dx > abs_dy) {
            if (abs_dx <= distance_to_move) {
                enemy->position.x = target_pos.x;
            } else {
                enemy->position.x += (direction.x > 0 ? 1.0f : -1.0f) * distance_to_move;
            }
        } else {
            if (abs_dy <= distance_to_move) {
                enemy->position.y = target_pos.y;
            } else {
                enemy->position.y += (direction.y > 0 ? 1.0f : -1.0f) * distance_to_move;
            }
        }
    }
}

SPRITE_INFO get_enemy_sprites(const ENEMY_TYPE type) {
    static constexpr int scout_sprites[] = { 0 };
    static constexpr int normal_sprites[] = { 1 };
    static constexpr int tank_sprites[] = { 2 };
    static constexpr int speedy_sprites[] = { 3 };

    SPRITE_INFO info = { .sprites = nullptr, .count = 0, .width = 0, .height = 0 };

    switch (type) {
        case ENEMY_TYPE_SCOUT:
            info.sprites = scout_sprites;
            info.count = sizeof(scout_sprites) / sizeof(scout_sprites[0]);
            break;
        case ENEMY_TYPE_NORMAL:
            info.sprites = normal_sprites;
            info.count = sizeof(normal_sprites) / sizeof(normal_sprites[0]);
            break;
        case ENEMY_TYPE_TANK:
            info.sprites = tank_sprites;
            info.count = sizeof(tank_sprites) / sizeof(tank_sprites[0]);
            break;
        case ENEMY_TYPE_SPEEDY:
            info.sprites = speedy_sprites;
            info.count = sizeof(speedy_sprites) / sizeof(speedy_sprites[0]);
            break;
        default:
            fprintf(stderr, "WARNING: Unknown enemy type\n");
            info.sprites = normal_sprites;
            info.count = 1;
    }

    info.width = 1;
    info.height = 1;
    return info;
}
