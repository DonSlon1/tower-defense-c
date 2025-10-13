//
// Created by lukas on 10/13/25.
//

#include "renderer.h"
#include "game.h"
#include "tower.h"
#include "enemy.h"
#include <stdio.h>

void draw_fullscreen_image(Texture2D texture) {
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();
    const float scale = (float)screen_width / (float)texture.width;
    const int draw_height = (int)((float)texture.height * scale);
    const int y = (screen_height - draw_height) / 2;

    float src_height = (float)texture.height;
    float src_y = 0;

    if (draw_height > screen_height) {
        src_height = (float)screen_height / scale;
        src_y = ((float)texture.height - src_height) / 2;
    }

    DrawTexturePro(
        texture,
        (Rectangle){0, src_y, (float)texture.width, src_height},
        (Rectangle){0, (float)y, (float)screen_width, (float)draw_height},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void draw_text_with_shadow(const char* text, const int x, const int y, const int size, const Color color) {
    DrawText(text, x + 2, y + 2, size, BLACK);
    DrawText(text, x, y, size, color);
}

void draw_centered_text_with_shadow(const char* text, const int y, const int size, const Color color) {
    const int screen_width = GetScreenWidth();
    const int text_width = MeasureText(text, size);
    const int text_x = (screen_width - text_width) / 2;
    draw_text_with_shadow(text, text_x, y, size, color);
}

void draw_game_objects(const GAME* game) {
    for (size_t i = 0; i < game->object_count; i++) {
        const GAME_OBJECT* obj = &game->game_objects[i];

        if (obj->type == TOWER) {
            const SPRITE_INFO info = get_tower_sprites(obj->data.tower.level);

            if (info.sprites == nullptr) {
                continue;
            }

            for (int y = 0; y < info.height; y++) {
                for (int x = 0; x < info.width; x++) {
                    const int sprite = info.sprites[info.width * y + x];
                    draw_texture(&game->tilemap, game->assets.towers, sprite, (int)obj->position.x + x, (int)obj->position.y + y);
                }
            }
        }
        if (obj->type == ENEMY) {
            const SPRITE_INFO info = get_enemy_sprites(obj->data.enemy.type);

            if (info.sprites == nullptr) {
                continue;
            }

            for (int y = 0; y < info.height; y++) {
                for (int x = 0; x < info.width; x++) {
                    const int sprite = info.sprites[info.width * y + x];
                    draw_texture(&game->tilemap, game->assets.enemies, sprite, (int)obj->position.x + x, (int)obj->position.y + y);
                }
            }
        }
    }
}

void draw_start_screen(const GAME* game) {
    draw_fullscreen_image(game->assets.start_screen);
    const int screen_height = GetScreenHeight();
    draw_centered_text_with_shadow("PRESS SPACE TO START", screen_height - 100, 40, WHITE);
}

void draw_game_over_screen(const GAME* game) {
    draw_fullscreen_image(game->assets.defeat_screen);

    const int screen_height = GetScreenHeight();

    char stats_text[256];
    snprintf(stats_text, sizeof(stats_text), "Enemies Defeated: %d", game->enemies_defeated);
    draw_centered_text_with_shadow(stats_text, screen_height / 2 + 50, 35, WHITE);

    char money_text[256];
    snprintf(money_text, sizeof(money_text), "Final Money: $%d", game->player_money);
    draw_centered_text_with_shadow(money_text, screen_height / 2 + 95, 35, GOLD);

    draw_centered_text_with_shadow("PRESS SPACE TO RESTART", screen_height - 80, 30, LIGHTGRAY);
}

void draw_hud(const GAME* game) {
    const int screen_width = GetScreenWidth();

    DrawRectangle(0, 0, screen_width, 50, (Color){0, 0, 0, 180});

    char lives_text[64];
    snprintf(lives_text, sizeof(lives_text), "Lives: %d", game->player_lives);
    DrawText(lives_text, 20, 15, 25, RED);

    char money_text[64];
    snprintf(money_text, sizeof(money_text), "$%d", game->player_money);
    const int money_x = screen_width / 3;
    DrawText(money_text, money_x, 15, 25, GOLD);

    char score_text[64];
    snprintf(score_text, sizeof(score_text), "Score: %d", game->enemies_defeated);
    const int score_x = screen_width * 2 / 3;
    DrawText(score_text, score_x, 15, 25, SKYBLUE);
}
