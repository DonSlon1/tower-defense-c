//
// Created by lukas on 10/12/25.
//

#ifndef PROJEKT_GAME_H
#define PROJEKT_GAME_H
#include <stddef.h>

#include "tilemap.h"

#define STARTING_COUNT_OF_GAME_OBJECTS 32
#define STARTING_AMOUNT_OF_MONEY 1000
#define STARTING_AMOUNT_OF_LIVES 100

// Tower default stats for LEVEL_0
#define TOWER_LEVEL_0_DAMAGE 0.0f
#define TOWER_LEVEL_0_RANGE 0.0f
#define TOWER_LEVEL_0_FIRE_COOLDOWN 1000.0f
#define TOWER_LEVEL_0_WIDTH 4
#define TOWER_LEVEL_0_HEIGHT 4
#define TOWER_LEVEL_0_UPGRADE_COST 100


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
} TowerSpriteInfo;

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
    float speed;
    int waypoint_index;
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

    union {
        TOWER_DATA tower;
        ENEMY_DATA enemy;
        ANIMATION_DATA animation;
    } data;

} GAME_OBJECT;

typedef struct {
    Texture2D towers;
} ASSETS;

typedef struct {
    GAME_OBJECT *game_objects;
    TILE_MAP tilemap;
    ASSETS assets;

    size_t object_count;
    size_t object_capacity;

    int player_lives;
    int player_money;
    int next_id;

} GAME;

GAME init_game();
void add_game_object(GAME *game,GAME_OBJECT game_object);
void grow_object_capacity(GAME *game);
void start_game(GAME *game);
void unload_game(GAME *game);
GAME_OBJECT init_tower(Vector2 position);
GRID_COORD screen_to_grid(Vector2 screen_pos, const TILE_MAP* tilemap);
int get_game_objects_of_type(const GAME *game, OBJECT_TYPE type, GAME_OBJECT **out_objects);
UPGRADE_RESULT upgrade_clicked_tower(GAME *game, GRID_COORD grid_coord);
TowerSpriteInfo get_tower_sprites(TOWER_LEVEL level);
void draw_game_objects(const GAME* game);

#endif //PROJEKT_GAME_H