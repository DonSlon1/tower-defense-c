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

upgrade_result upgrade_clicked_tower(game *game, const grid_coord grid_coord) {
    if (game == NULL || game->game_objects == NULL) {
        return upgrade_not_found;
    }

    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type != tower) {
            continue;
        }
        game_object* tower = &game->game_objects[i];

        const int tower_grid_x = (int)tower->position.x;
        const int tower_grid_y = (int)tower->position.y;
        const int tower_width  = tower->data.tower.width;
        const int tower_height = tower->data.tower.height;

        const bool is_inside = grid_coord.x >= tower_grid_x &&
                               grid_coord.x < tower_grid_x + tower_width &&
                               grid_coord.y >= tower_grid_y &&
                               grid_coord.y < tower_grid_y + tower_height;

        if (is_inside) {
            if (tower->data.tower.level >= level_1) {
                return upgrade_max_level;
            }

            if (game->player_money < tower->data.tower.upgrade_cost) {
                return upgrade_insufficient_funds;
            }

            tower->data.tower.level++;
            game->player_money -= tower->data.tower.upgrade_cost;

            if (tower->data.tower.level == level_1) {
                tower->data.tower.damage = TOWER_LEVEL_1_DAMAGE;
                tower->data.tower.range = TOWER_LEVEL_1_RANGE;
                tower->data.tower.fire_cooldown = TOWER_LEVEL_1_FIRE_COOLDOWN;
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
    sprite_info info = { .sprites = NULL, .count = 0, .width = 0, .height = 0 };

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
        default:
            fprintf(stderr, "WARNING: Failed to find sprite for tower level\n");
    }
    return info;
}

int find_nearest_enemy_in_range(const game *game, const vector2 tower_pos, const float range) {
    if (game == NULL || game->game_objects == NULL) {
        return -1;
    }

    int nearest_id = -1;
    float nearest_dist_sq = range * range;

    const vector2 tower_center = {tower_pos.x + 2.0f, tower_pos.y + 2.0f};

    for (size_t i = 0; i < game->object_count; i++) {
        const game_object* obj = &game->game_objects[i];

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

void update_tower(game *game, game_object *tower, const float delta_time) {
    if (game == NULL || tower == NULL) return;

    if (!tower->is_active || tower->data.tower.level == level_0) {
        return;
    }

    if (tower->data.tower.fire_cooldown > 0) {
        tower->data.tower.fire_cooldown -= delta_time;
    }

    const vector2 tower_pos = tower->position;
    const int target_id = find_nearest_enemy_in_range(game, tower_pos, tower->data.tower.range);

    if (target_id == -1) {
        tower->data.tower.target_id = -1;
        return;
    }

    tower->data.tower.target_id = target_id;

    if (tower->data.tower.fire_cooldown <= 0) {
        for (size_t i = 0; i < game->object_count; i++) {
            if (game->game_objects[i].id == target_id && game->game_objects[i].is_active) {
                const vector2 tower_center = {tower_pos.x + 2.0f, tower_pos.y + 2.0f};
                const vector2 target_pos = game->game_objects[i].position;

                const game_object projectile = create_projectile(
                    tower_center,
                    target_pos,
                    tower->data.tower.damage,
                    tower->id,
                    target_id
                );

                add_game_object(game, projectile);

                tower->data.tower.fire_cooldown = TOWER_LEVEL_1_FIRE_COOLDOWN;
                break;
            }
        }
    }
}
