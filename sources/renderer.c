//
// Created by lukas on 10/13/25.
//

#include "renderer.h"
#include "game.h"
#include "tower.h"
#include "enemy.h"
#include <stdio.h>
#include <math.h>

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

Texture2D get_enemy_texture(const GAME* game, const ENEMY_TYPE type, const ENEMY_ANIMATION_STATE state) {
    switch (type) {
        case ENEMY_TYPE_MUSHROOM:
            switch (state) {
                case ENEMY_ANIM_RUN: return game->assets.mushroom_run;
                case ENEMY_ANIM_HIT: return game->assets.mushroom_hit;
                case ENEMY_ANIM_DIE: return game->assets.mushroom_die;
            }
            break;
        case ENEMY_TYPE_FLYING:
            switch (state) {
                case ENEMY_ANIM_RUN: return game->assets.flying_fly;
                case ENEMY_ANIM_HIT: return game->assets.flying_hit;
                case ENEMY_ANIM_DIE: return game->assets.flying_die;
            }
            break;
        default:
            return game->assets.mushroom_run;
    }
    return game->assets.mushroom_run;
}

void draw_game_objects(const GAME* game) {
    const int tile_size = get_tile_scale(&game->tilemap);

    for (size_t i = 0; i < game->object_count; i++) {
        const GAME_OBJECT* obj = &game->game_objects[i];

        if (!obj->is_active) continue;

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
        else if (obj->type == ENEMY) {
            const Texture2D texture = get_enemy_texture(game, obj->data.enemy.type, obj->data.enemy.anim_state);
            const int frame_count = get_enemy_frame_count(obj->data.enemy.type, obj->data.enemy.anim_state);

            if (texture.id == 0 || frame_count == 0) {
                continue;
            }

            const int frame_width = texture.width / frame_count;
            const int frame_height = texture.height;

            const Rectangle source = {
                (float)(obj->data.enemy.current_frame * frame_width),
                0,
                (float)frame_width,
                (float)frame_height
            };

            const float aspect_ratio = (float)frame_width / (float)frame_height;
            const float draw_width = (float)tile_size;
            const float draw_height = draw_width / aspect_ratio;

            float offset_y = 0;
            if (draw_height < tile_size) {
                offset_y = ((float)tile_size - draw_height) / 2.0f;
            }

            const Rectangle dest = {
                obj->position.x * (float)tile_size,
                obj->position.y * (float)tile_size + offset_y,
                draw_width,
                draw_height
            };

            DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        }
        else if (obj->type == PROJECTILE) {
            const Texture2D iceball = game->assets.iceball;

            if (iceball.id == 0) {
                continue;
            }

            constexpr int frame_width = 84;
            constexpr int frame_height = 9;

            const int current_frame = obj->data.projectile.current_frame;

            const Rectangle source = {
                (float)(current_frame * frame_width),
                0,
                (float)frame_width,
                (float)frame_height
            };

            const float projectile_width = (float)tile_size * 2.0f;  // 4x the original 0.5 scale
            const float projectile_height = projectile_width * ((float)frame_height / (float)frame_width);

            const float angle = atan2f(obj->data.projectile.velocity.y, obj->data.projectile.velocity.x) * (180.0f / 3.14159f) + 180.0f;

            const Rectangle dest = {
                obj->position.x * (float)tile_size,
                obj->position.y * (float)tile_size,
                projectile_width,
                projectile_height
            };

            const Vector2 origin = {projectile_width / 2.0f, projectile_height / 2.0f};

            DrawTexturePro(iceball, source, dest, origin, angle, WHITE);
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

void draw_wave_info(const GAME* game) {
    const int screen_width = GetScreenWidth();

    char wave_text[64];
    snprintf(wave_text, sizeof(wave_text), "Wave %d", game->current_wave + 1);

    constexpr int font_size = 30;
    const int text_width = MeasureText(wave_text, font_size);
    const int x = screen_width - text_width - 20;
    constexpr int y = 60;

    DrawRectangle(x - 10, y - 5, text_width + 20, font_size + 10, (Color){0, 0, 0, 180});

    DrawText(wave_text, x, y, font_size, (Color){255, 200, 0, 255});

    const WAVE_CONFIG wave = get_wave_config(game->current_wave);
    char progress_text[64];
    snprintf(progress_text, sizeof(progress_text), "%d/%d enemies",
             game->enemies_spawned_in_wave, wave.enemy_count);

    constexpr int progress_font_size = 18;
    const int progress_width = MeasureText(progress_text, progress_font_size);
    const int progress_x = screen_width - progress_width - 20;
    constexpr int progress_y = y + font_size + 10;

    DrawText(progress_text, progress_x, progress_y, progress_font_size, (Color){200, 200, 200, 255});
}

void draw_wave_break_screen(const GAME* game) {
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();

    DrawRectangle(0, 0, screen_width, screen_height, (Color){0, 0, 0, 150});

    char complete_text[64];
    snprintf(complete_text, sizeof(complete_text), "WAVE %d COMPLETE!", game->current_wave + 1);
    draw_centered_text_with_shadow(complete_text, screen_height / 2 - 80, 50, GREEN);

    const WAVE_CONFIG next_wave = get_wave_config(game->current_wave + 1);
    char next_wave_text[128];
    snprintf(next_wave_text, sizeof(next_wave_text), "Next: Wave %d (%d enemies)",
             game->current_wave + 2, next_wave.enemy_count);
    draw_centered_text_with_shadow(next_wave_text, screen_height / 2 - 20, 30, SKYBLUE);

    char timer_text[64];
    snprintf(timer_text, sizeof(timer_text), "Starting in %.0f seconds...", game->wave_break_timer);
    draw_centered_text_with_shadow(timer_text, screen_height / 2 + 20, 25, WHITE);

    draw_centered_text_with_shadow("PRESS SPACE TO START NOW", screen_height / 2 + 60, 20, LIGHTGRAY);
}

void draw_tower_info(const GAME_OBJECT* tower, const int x, const int y) {
    if (tower == nullptr || tower->type != TOWER) {
        return;
    }

    if (tower->data.tower.level <= LEVEL_0) {
        return;
    }

    const char* tower_name = "Ice Tower";
    char level_text[32];
    snprintf(level_text, sizeof(level_text), "Level %d", tower->data.tower.level);

    constexpr int padding = 10;
    constexpr int font_size = 20;
    const int name_width = MeasureText(tower_name, font_size);
    const int level_width = MeasureText(level_text, font_size);
    const int max_width = (name_width > level_width) ? name_width : level_width;

    const int box_width = max_width + padding * 2;
    constexpr int box_height = font_size * 2 + padding * 3;

    DrawRectangle(x, y, box_width, box_height, (Color){0, 0, 0, 200});
    DrawRectangleLines(x, y, box_width, box_height, (Color){100, 200, 255, 255});

    DrawText(tower_name, x + padding, y + padding, font_size, SKYBLUE);

    DrawText(level_text, x + padding, y + padding * 2 + font_size, font_size, GOLD);
}

void draw_tower_spots(const GAME* game) {
    const int tile_size = get_tile_scale(&game->tilemap);

    for (int i = 0; i < 4; i++) {
        const TOWER_SPOT* spot = &game->tower_spots[i];

        if (!spot->occupied) {
            const int x = (int)spot->position.x * tile_size;
            const int y = (int)spot->position.y * tile_size;
            const int size = 4 * tile_size;

            DrawRectangle(x, y, size, size, (Color){0, 255, 0, 50});
            DrawRectangleLines(x, y, size, size, (Color){0, 255, 0, 150});

            const char* cost_text = "$100";
            const int text_width = MeasureText(cost_text, 20);
            DrawText(cost_text, x + (size - text_width) / 2, y + size / 2 - 10, 20, (Color){255, 255, 255, 200});
        }
    }
}
