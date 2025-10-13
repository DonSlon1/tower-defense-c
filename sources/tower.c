//
// Created by lukas on 10/13/25.
//

#include "tower.h"
#include "game.h"
#include <stdio.h>

GAME_OBJECT init_tower(const Vector2 position) {
    return (GAME_OBJECT) {
        .type = TOWER,
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
            .level = LEVEL_0
        }
    };
}

UPGRADE_RESULT upgrade_clicked_tower(GAME *game, const GRID_COORD grid_coord) {
    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type != TOWER) {
            continue;
        }
        GAME_OBJECT* tower = &game->game_objects[i];

        const int tower_grid_x = (int)tower->position.x;
        const int tower_grid_y = (int)tower->position.y;
        const int tower_width  = tower->data.tower.width;
        const int tower_height = tower->data.tower.height;

        const bool is_inside = grid_coord.x >= tower_grid_x &&
                               grid_coord.x < tower_grid_x + tower_width &&
                               grid_coord.y >= tower_grid_y &&
                               grid_coord.y < tower_grid_y + tower_height;

        if (is_inside) {
            if (tower->data.tower.level >= LEVEL_1) {
                return UPGRADE_MAX_LEVEL;
            }

            if (game->player_money < tower->data.tower.upgrade_cost) {
                return UPGRADE_INSUFFICIENT_FUNDS;
            }

            tower->data.tower.level++;
            game->player_money -= tower->data.tower.upgrade_cost;
            return UPGRADE_SUCCESS;
        }
    }
    return UPGRADE_NOT_FOUND;
}

SPRITE_INFO get_tower_sprites(const TOWER_LEVEL level) {
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
    SPRITE_INFO info = { .sprites = nullptr, .count = 0, .width = 0, .height = 0 };

    switch (level) {
        case LEVEL_0:
            info.sprites = level_0_sprites;
            info.count = sizeof(level_0_sprites) / sizeof(level_0_sprites[0]);
            info.width = 4;
            info.height = 4;
            break;
        case LEVEL_1:
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
