//
// Created by lukas on 10/13/25.
//

#include "tower.h"
#include "game.h"
#include "projectile.h"
#include <stdio.h>

typedef game game;

game_object init_tower(const vector2 position) {
    return (game_object) {
        .type = tower,
        .position = position,
        .is_active = true,
        .data.tower = {
            .damage = TOWER_LEVEL_0_DAMAGE,
            .range = TOWER_LEVEL_0_RANGE,
            .fire_cooldown = TOWER_LEVEL_0_FIRE_COOLDOWN,
            .target_id = -1,
            .width = TOWER_LEVEL_0_WIDTH,
            .height = TOWER_LEVEL_0_HEIGHT,
            .upgrade_cost = TOWER_LEVEL_0_UPGRADE_COST,
            .level = level_0
        }
    };
}

upgrade_result upgrade_clicked_tower(game *g, const grid_coord coord) {
    if (g == nullptr || g->game_objects == nullptr) {
        return upgrade_not_found;
    }

    for (size_t i = 0; i < g->object_count; i++) {
        if (g->game_objects[i].type != tower) {
            continue;
        }
        game_object* twr = &g->game_objects[i];

        const int tower_grid_x = (int)twr->position.x;
        const int tower_grid_y = (int)twr->position.y;
        const int tower_width  = twr->data.tower.width;
        const int tower_height = twr->data.tower.height;

        const bool is_inside = coord.x >= tower_grid_x &&
                               coord.x < tower_grid_x + tower_width &&
                               coord.y >= tower_grid_y &&
                               coord.y < tower_grid_y + tower_height;

        if (is_inside) {
            if (twr->data.tower.level >= level_1) {
                return upgrade_max_level;
            }

            if (g->player_money < twr->data.tower.upgrade_cost) {
                return upgrade_insufficient_funds;
            }

            twr->data.tower.level++;
            g->player_money -= twr->data.tower.upgrade_cost;

            if (twr->data.tower.level == level_1) {
                twr->data.tower.damage = TOWER_LEVEL_1_DAMAGE;
                twr->data.tower.range = TOWER_LEVEL_1_RANGE;
                twr->data.tower.fire_cooldown = TOWER_LEVEL_1_FIRE_COOLDOWN;
            }

            return upgrade_success;
        }
    }
    return upgrade_not_found;
}

sprite_info get_tower_sprites(const tower_level level) {
    static const int level_0_sprites[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    };
    static const int level_1_sprites[] = {
        16, 17, 18, 19,
        20, 21, 22, 23,
        24, 25, 26, 27,
        28, 29, 30, 31
    };
    sprite_info info = { .sprites = nullptr, .count = 0, .width = 0, .height = 0 };

    switch (level) {
        case level_0:
            info.sprites = level_0_sprites;
            info.count = sizeof(level_0_sprites) / sizeof(level_0_sprites[0]);
            info.width = 4;
            info.height = 4;
            break;
        case level_1:
            info.sprites = level_1_sprites;
            info.count = sizeof(level_1_sprites) / sizeof(level_1_sprites[0]);
            info.width = 4;
            info.height = 4;
            break;
        case level_max:
        default:
            fprintf(stderr, "WARNING: Failed to find sprite for tower level\n");
    }
    return info;
}

int find_nearest_enemy_in_range(const game *g, const vector2 tower_pos, const float range) {
    if (g == nullptr || g->game_objects == nullptr) {
        return -1;
    }

    int nearest_id = -1;
    float nearest_dist_sq = range * range;

    const vector2 tower_center = {tower_pos.x + 2.0f, tower_pos.y + 2.0f};

    for (size_t i = 0; i < g->object_count; i++) {
        const game_object* obj = &g->game_objects[i];

        if (obj->type != enemy || !obj->is_active) {
            continue;
        }

        if (obj->data.enemy.anim_state == enemy_anim_die) {
            continue;
        }

        const float dx = obj->position.x - tower_center.x;
        const float dy = obj->position.y - tower_center.y;
        const float dist_sq = dx * dx + dy * dy;

        if (dist_sq < nearest_dist_sq) {
            nearest_dist_sq = dist_sq;
            nearest_id = obj->id;
        }
    }

    return nearest_id;
}

void update_tower(game *g, game_object *twr, const float delta_time) {
    if (g == nullptr || twr == nullptr) return;

    if (!twr->is_active || twr->data.tower.level == level_0) {
        return;
    }

    if (twr->data.tower.fire_cooldown > 0) {
        twr->data.tower.fire_cooldown -= delta_time;
    }

    const vector2 tower_pos = twr->position;
    const int target_id = find_nearest_enemy_in_range(g, tower_pos, twr->data.tower.range);

    if (target_id == -1) {
        twr->data.tower.target_id = -1;
        return;
    }

    twr->data.tower.target_id = target_id;

    if (twr->data.tower.fire_cooldown <= 0) {
        for (size_t i = 0; i < g->object_count; i++) {
            if (g->game_objects[i].id == target_id && g->game_objects[i].is_active) {
                const vector2 tower_center = {tower_pos.x + 2.0f, tower_pos.y + 2.0f};
                const vector2 target_pos = g->game_objects[i].position;

                const game_object proj = create_projectile(
                    tower_center,
                    target_pos,
                    twr->data.tower.damage,
                    twr->id,
                    target_id
                );

                add_game_object(g, proj);

                twr->data.tower.fire_cooldown = TOWER_LEVEL_1_FIRE_COOLDOWN;
                break;
            }
        }
    }
}
