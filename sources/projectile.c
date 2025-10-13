//
// Created by lukas on 10/13/25.
//

#include "projectile.h"
#include "game.h"
#include <math.h>
#include <stdio.h>

#define PROJECTILE_SPEED 10.0f
#define ICEBALL_FRAMES 10
#define ICEBALL_FRAME_DURATION 0.05f

GAME_OBJECT create_projectile(Vector2 start_pos, Vector2 target_pos, float damage, int owner_id, int target_id) {
    Vector2 direction = {
        target_pos.x - start_pos.x,
        target_pos.y - start_pos.y
    };

    const float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.001f) {
        direction.x /= length;
        direction.y /= length;
    }

    const Vector2 velocity = {
        direction.x * PROJECTILE_SPEED,
        direction.y * PROJECTILE_SPEED
    };

    return (GAME_OBJECT) {
        .type = PROJECTILE,
        .position = start_pos,
        .is_active = true,
        .data.projectile = {
            .velocity = velocity,
            .damage = damage,
            .owner_id = owner_id,
            .target_id = target_id,
            .current_frame = 0,
            .frame_timer = 0.0f,
            .row = 0
        }
    };
}

void update_projectile(const GAME *game, GAME_OBJECT *projectile, const float delta_time) {
    if (game == nullptr || projectile == nullptr || game->game_objects == nullptr) return;

    if (!projectile->is_active) {
        return;
    }

    projectile->data.projectile.frame_timer += delta_time;
    if (projectile->data.projectile.frame_timer >= ICEBALL_FRAME_DURATION) {
        projectile->data.projectile.frame_timer = 0.0f;
        projectile->data.projectile.current_frame++;
        if (projectile->data.projectile.current_frame >= ICEBALL_FRAMES) {
            projectile->data.projectile.current_frame = 0;
        }
    }

    const int target_id = projectile->data.projectile.target_id;
    GAME_OBJECT* target = nullptr;

    for (size_t i = 0; i < game->object_count; i++) {
        GAME_OBJECT* obj = &game->game_objects[i];

        if (obj->type != ENEMY || obj->id != target_id) {
            continue;
        }

        if (obj->is_active && obj->data.enemy.anim_state != ENEMY_ANIM_DIE) {
            target = obj;
            break;
        }
    }

    if (target != nullptr) {
        const float dx = target->position.x - projectile->position.x;
        const float dy = target->position.y - projectile->position.y;

        const float length = sqrtf(dx * dx + dy * dy);
        if (length > 0.001f) {
            const float inv_length = 1.0f / length;
            projectile->data.projectile.velocity.x = dx * inv_length * PROJECTILE_SPEED;
            projectile->data.projectile.velocity.y = dy * inv_length * PROJECTILE_SPEED;
        }
    }

    projectile->position.x += projectile->data.projectile.velocity.x * delta_time;
    projectile->position.y += projectile->data.projectile.velocity.y * delta_time;

    if (projectile->position.x < -2 || projectile->position.x > 26 ||
        projectile->position.y < -2 || projectile->position.y > 20) {
        projectile->is_active = false;
        return;
    }

    constexpr float collision_dist_sq = 1.0f;

    if (target != nullptr) {
        const float dx = target->position.x - projectile->position.x;
        const float dy = target->position.y - projectile->position.y;

        if (dx * dx + dy * dy < collision_dist_sq) {
            target->data.enemy.health -= projectile->data.projectile.damage;

            if (target->data.enemy.health > 0) {
                target->data.enemy.anim_state = ENEMY_ANIM_HIT;
                target->data.enemy.current_frame = 0;
                target->data.enemy.frame_timer = 0.0f;
            }

            projectile->is_active = false;
            return;
        }
    }

    for (size_t i = 0; i < game->object_count; i++) {
        GAME_OBJECT* obj = &game->game_objects[i];

        if (obj->type != ENEMY || !obj->is_active || obj->data.enemy.anim_state == ENEMY_ANIM_DIE) {
            continue;
        }

        if (obj->id == target_id) {
            continue;
        }

        const float dx = obj->position.x - projectile->position.x;
        const float dy = obj->position.y - projectile->position.y;

        if (fabsf(dx) > 1.0f || fabsf(dy) > 1.0f) {
            continue;
        }

        if (dx * dx + dy * dy < collision_dist_sq) {
            obj->data.enemy.health -= projectile->data.projectile.damage;

            if (obj->data.enemy.health > 0) {
                obj->data.enemy.anim_state = ENEMY_ANIM_HIT;
                obj->data.enemy.current_frame = 0;
                obj->data.enemy.frame_timer = 0.0f;
            }

            projectile->is_active = false;
            return;
        }
    }
}
