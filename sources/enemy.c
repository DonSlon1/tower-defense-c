#include "enemy.h"
#include <stdio.h>
#include <math.h>

constexpr int MUSHROOM_RUN_FRAMES = 8;
constexpr int MUSHROOM_HIT_FRAMES = 5;
constexpr int MUSHROOM_DIE_FRAMES = 15;
constexpr int FLYING_FLY_FRAMES = 8;
constexpr int FLYING_HIT_FRAMES = 4;
constexpr int FLYING_DIE_FRAMES = 17;
constexpr float ANIM_FRAME_DURATION = 0.1f;
constexpr float WAYPOINT_REACHED_THRESHOLD = 0.01f;

static constexpr Vector2 path_0_waypoints[] = {
    {0, 2},
    {19, 2},
    {19, 7},
    {24, 7}
};

static constexpr Vector2 path_1_waypoints[] = {
    {0, 15},
    {19, 15},
    {19, 10},
    {24, 10}
};

static constexpr int path_0_count = sizeof(path_0_waypoints) / sizeof(path_0_waypoints[0]);
static constexpr int path_1_count = sizeof(path_1_waypoints) / sizeof(path_1_waypoints[0]);

static Vector2* const paths[] = { (Vector2*)path_0_waypoints, (Vector2*)path_1_waypoints };
static constexpr int path_counts[] = { path_0_count, path_1_count };

ENEMY_STATS get_enemy_stats(const ENEMY_TYPE type) {
    switch (type) {
        case ENEMY_TYPE_MUSHROOM:
            return (ENEMY_STATS) { .health = 80.0f, .speed = 1.5f, .gold_reward = 25 };
        case ENEMY_TYPE_FLYING:
            return (ENEMY_STATS) { .health = 50.0f, .speed = 3.5f, .gold_reward = 15 };
        default:
            return (ENEMY_STATS) { .health = 80.0f, .speed = 1.5f, .gold_reward = 25 };
    }
}

int get_enemy_frame_count(const ENEMY_TYPE type, const ENEMY_ANIMATION_STATE state) {
    switch (type) {
        case ENEMY_TYPE_MUSHROOM:
            switch (state) {
                case ENEMY_ANIM_RUN: return MUSHROOM_RUN_FRAMES;
                case ENEMY_ANIM_HIT: return MUSHROOM_HIT_FRAMES;
                case ENEMY_ANIM_DIE: return MUSHROOM_DIE_FRAMES;
            }
            break;
        case ENEMY_TYPE_FLYING:
            switch (state) {
                case ENEMY_ANIM_RUN: return FLYING_FLY_FRAMES;
                case ENEMY_ANIM_HIT: return FLYING_HIT_FRAMES;
                case ENEMY_ANIM_DIE: return FLYING_DIE_FRAMES;
            }
            break;
        default:
            return 1;
    }
    return 1;
}

void update_enemy_animation(GAME_OBJECT* const enemy, const float delta_time) {
    enemy->data.enemy.frame_timer += delta_time;

    if (enemy->data.enemy.frame_timer >= ANIM_FRAME_DURATION) {
        enemy->data.enemy.frame_timer = 0.0f;
        enemy->data.enemy.current_frame++;

        const int max_frames = get_enemy_frame_count(enemy->data.enemy.type, enemy->data.enemy.anim_state);

        if (enemy->data.enemy.anim_state == ENEMY_ANIM_DIE) {
            if (enemy->data.enemy.current_frame >= max_frames) {
                enemy->data.enemy.current_frame = max_frames - 1;
                enemy->is_active = false;
            }
        } else if (enemy->data.enemy.anim_state == ENEMY_ANIM_HIT) {
            if (enemy->data.enemy.current_frame >= max_frames) {
                enemy->data.enemy.anim_state = ENEMY_ANIM_RUN;
                enemy->data.enemy.current_frame = 0;
            }
        } else {
            if (enemy->data.enemy.current_frame >= max_frames) {
                enemy->data.enemy.current_frame = 0;
            }
        }
    }
}

void update_enemy(GAME_OBJECT* const enemy, const float delta_time) {
    if (!enemy->is_active) {
        return;
    }

    update_enemy_animation(enemy, delta_time);

    if (enemy->data.enemy.anim_state == ENEMY_ANIM_DIE) {
        return;
    }

    if (enemy->data.enemy.health <= 0) {
        enemy->data.enemy.anim_state = ENEMY_ANIM_DIE;
        enemy->data.enemy.current_frame = 0;
        enemy->data.enemy.frame_timer = 0.0f;
        return;
    }

    const int path_id = enemy->data.enemy.path_id;
    const int waypoint_count = path_counts[path_id];
    const Vector2* const current_path = paths[path_id];

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

    if (distance < WAYPOINT_REACHED_THRESHOLD) {
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

SPRITE_INFO get_enemy_sprites(const ENEMY_TYPE type, const ENEMY_ANIMATION_STATE state) {
    SPRITE_INFO info = { .sprites = nullptr, .count = 0, .width = 1, .height = 1 };

    switch (type) {
        case ENEMY_TYPE_MUSHROOM:
        case ENEMY_TYPE_FLYING:
            info.count = get_enemy_frame_count(type, state);
            break;
        default:
            fprintf(stderr, "ERROR: Unknown enemy type\n");
            info.count = 1;
    }

    return info;
}

Vector2 get_path_start_position(const int path_id) {
    return path_id == 0 ? path_0_waypoints[0] : path_1_waypoints[0];
}
