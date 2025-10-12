//
// Created by lukas on 10/12/25.
//

#ifndef PROJEKT_GAME_H
#define PROJEKT_GAME_H
#include "tilemap.h"

#define STARTING_COUNT_OF_GAME_OBJECTS 32


typedef enum {
    TOWER,
    ENEMY,
    ANIMATION,
    PROJECTILE
} OBJECT_TYPE;

typedef enum {
    LEVEL_0,
    LEVEL_1
} TOWER_LEVEL;

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
    float fire_cooldown;
    int target_id;
    int width;
    int height;
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

    int object_count;
    int object_capacity;

    int player_lives;
    int player_money;
    int next_id;

} GAME;

GAME init_game();
void add_game_object(GAME *game,GAME_OBJECT game_object);
void add_object_size(GAME *game);
void start_game(GAME *game);
void unload_game(GAME *game);
GAME_OBJECT init_tower(Vector2 position, const GAME *game);
GRID_COORD screen_to_grid(Vector2 screen_pos, const TILE_MAP* tilemap);
int get_game_objects_of_type(const GAME *game, OBJECT_TYPE type, GAME_OBJECT **out_objects);
bool upgrade_clicked_tower(GAME *game, GRID_COORD grid_coord);
const int* get_tower_sprites(TOWER_LEVEL level, int* out_count);
void draw_game_objects(const GAME* game);

#endif //PROJEKT_GAME_H