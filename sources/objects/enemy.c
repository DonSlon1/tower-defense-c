#include "enemy.h"
#include <stdio.h>
#include <math.h>

constexpr int mushroom_run_frames = 8;
constexpr int mushroom_hit_frames = 5;
constexpr int mushroom_die_frames = 15;
constexpr int flying_fly_frames = 8;
constexpr int flying_hit_frames = 4;
constexpr int flying_die_frames = 17;
constexpr float anim_frame_duration = 0.1f;
constexpr float waypoint_reached_threshold = 0.01f;

static const vector2 path_0_waypoints[] = {
    {0, 2},
    {19, 2},
    {19, 7},
    {24, 7}
};

static const vector2 path_1_waypoints[] = {
    {0, 15},
    {19, 15},
    {19, 10},
    {24, 10}
};

static constexpr int path_0_count = sizeof(path_0_waypoints) / sizeof(path_0_waypoints[0]);
static constexpr int path_1_count = sizeof(path_1_waypoints) / sizeof(path_1_waypoints[0]);

static const vector2* const paths[] = { path_0_waypoints, path_1_waypoints };
static constexpr int path_counts[] = { path_0_count, path_1_count };

enemy_stats get_enemy_stats(const enemy_type type) {
    switch (type) {
        case enemy_type_mushroom:
            return (enemy_stats) { .health = 80.0f, .speed = 1.5f, .gold_reward = 25 };
        case enemy_type_flying:
            return (enemy_stats) { .health = 50.0f, .speed = 3.5f, .gold_reward = 15 };
        case enemy_type_count:
        default:
            return (enemy_stats) { .health = 80.0f, .speed = 1.5f, .gold_reward = 25 };
    }
}

int get_enemy_frame_count(const enemy_type type, const enemy_animation_state state) {
    switch (type) {
        case enemy_type_mushroom:
            switch (state) {
                case enemy_anim_run: return mushroom_run_frames;
                case enemy_anim_hit: return mushroom_hit_frames;
                case enemy_anim_die: return mushroom_die_frames;
                default: return mushroom_run_frames;
            }
        case enemy_type_flying:
            switch (state) {
                case enemy_anim_run: return flying_fly_frames;
                case enemy_anim_hit: return flying_hit_frames;
                case enemy_anim_die: return flying_die_frames;
                default: return flying_fly_frames;
            }
        case enemy_type_count:
        default:
            return 1;
    }
}

void update_enemy_animation(game_object* const en, const float delta_time) {
    if (en == nullptr) return;

    en->data.enemy.frame_timer += delta_time;

    if (en->data.enemy.frame_timer >= anim_frame_duration) {
        en->data.enemy.frame_timer = 0.0f;
        en->data.enemy.current_frame++;

        const int max_frames = get_enemy_frame_count(en->data.enemy.type, en->data.enemy.anim_state);

        if (en->data.enemy.anim_state == enemy_anim_die) {
            if (en->data.enemy.current_frame >= max_frames) {
                en->data.enemy.current_frame = max_frames - 1;
                en->is_active = false;
            }
        } else if (en->data.enemy.anim_state == enemy_anim_hit) {
            if (en->data.enemy.current_frame >= max_frames) {
                en->data.enemy.anim_state = enemy_anim_run;
                en->data.enemy.current_frame = 0;
            }
        } else {
            if (en->data.enemy.current_frame >= max_frames) {
                en->data.enemy.current_frame = 0;
            }
        }
    }
}

void update_enemy(game_object* const en, const float delta_time) {
    if (en == nullptr) return;

    if (!en->is_active) {
        return;
    }

    update_enemy_animation(en, delta_time);

    if (en->data.enemy.anim_state == enemy_anim_die) {
        return;
    }

    if (en->data.enemy.health <= 0) {
        en->data.enemy.anim_state = enemy_anim_die;
        en->data.enemy.current_frame = 0;
        en->data.enemy.frame_timer = 0.0f;
        return;
    }

    const int path_id = en->data.enemy.path_id;

    if (path_id < 0 || path_id >= 2) {
        en->is_active = false;
        return;
    }

    const int waypoint_count = path_counts[path_id];
    const vector2* const current_path = paths[path_id];

    if (en->data.enemy.waypoint_index >= waypoint_count) {
        en->is_active = false;
        return;
    }

    const vector2 target_pos = current_path[en->data.enemy.waypoint_index];
    const vector2 direction = {
        target_pos.x - en->position.x,
        target_pos.y - en->position.y
    };

    const float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);

    if (distance < waypoint_reached_threshold) {
        en->position = target_pos;
        en->data.enemy.waypoint_index++;
    } else {
        const float abs_dx = fabsf(direction.x);
        const float abs_dy = fabsf(direction.y);
        const float distance_to_move = en->data.enemy.speed * delta_time;

        if (abs_dx > abs_dy) {
            if (abs_dx <= distance_to_move) {
                en->position.x = target_pos.x;
            } else {
                en->position.x += (direction.x > 0 ? 1.0f : -1.0f) * distance_to_move;
            }
        } else {
            if (abs_dy <= distance_to_move) {
                en->position.y = target_pos.y;
            } else {
                en->position.y += (direction.y > 0 ? 1.0f : -1.0f) * distance_to_move;
            }
        }
    }
}

sprite_info get_enemy_sprites(const enemy_type type, const enemy_animation_state state) {
    sprite_info info = { .sprites = nullptr, .count = 0, .width = 1, .height = 1 };

    switch (type) {
        case enemy_type_mushroom:
        case enemy_type_flying:
            info.count = get_enemy_frame_count(type, state);
            break;
        case enemy_type_count:
        default:
            fprintf(stderr, "ERROR: Unknown enemy type\n");
            info.count = 1;
    }

    return info;
}

vector2 get_path_start_position(const int path_id) {
    return path_id == 0 ? path_0_waypoints[0] : path_1_waypoints[0];
}
