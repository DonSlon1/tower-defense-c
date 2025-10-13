//
// Created by lukas on 10/12/25.
//

#ifndef PROJEKT_GAME_H
#define PROJEKT_GAME_H
#include <stddef.h>

#include "tilemap.h"
#include "game_object.h"

#define STARTING_COUNT_OF_GAME_OBJECTS 32
#define STARTING_AMOUNT_OF_MONEY 250
#define STARTING_AMOUNT_OF_LIVES 100
#define TOWER_BUILD_COST 100

#define MAX_WAVES 10
#define WAVE_BREAK_DURATION 10.0f

typedef struct {
    int enemy_count;
    float spawn_interval;
    int flying_chance;
    bool allow_bottom_path;
} WAVE_CONFIG;

typedef enum {
    GAME_STATE_START,
    GAME_STATE_PLAYING,
    GAME_STATE_WAVE_BREAK,
    GAME_STATE_GAME_OVER
} GAME_STATE;

typedef struct {
    Texture2D towers;
    Texture2D mushroom_run;
    Texture2D mushroom_hit;
    Texture2D mushroom_die;
    Texture2D flying_fly;
    Texture2D flying_hit;
    Texture2D flying_die;
    Texture2D iceball;
    Texture2D start_screen;
    Texture2D defeat_screen;
} ASSETS;

typedef struct {
    Vector2 position;
    bool occupied;
} TOWER_SPOT;

typedef struct GAME {
    GAME_OBJECT *game_objects;
    TILE_MAP tilemap;
    ASSETS assets;

    size_t object_count;
    size_t object_capacity;

    int player_lives;
    int player_money;
    int next_id;
    float enemy_spawn_timer;

    TOWER_SPOT tower_spots[4];

    int current_wave;
    int enemies_spawned_in_wave;
    int enemies_alive;
    float wave_break_timer;

    GAME_STATE state;
    int enemies_defeated;
} GAME;

GAME init_game();
void add_game_object(GAME *game, GAME_OBJECT game_object);
void grow_object_capacity(GAME *game);
void start_game(GAME *game);
void unload_game(GAME *game);
GRID_COORD screen_to_grid(Vector2 screen_pos, const TILE_MAP* tilemap);
int get_game_objects_of_type(const GAME *game, OBJECT_TYPE type, GAME_OBJECT **out_objects);
void update_game_state(GAME *game, float delta_time);
void remove_inactive_objects(GAME *game);
GAME_OBJECT* find_tower_at_grid(const GAME *game, GRID_COORD grid_coord);
int find_tower_spot_at_grid(const GAME *game, GRID_COORD grid_coord);
bool try_build_tower(GAME *game, int spot_index);
WAVE_CONFIG get_wave_config(int wave_number);
void start_next_wave(GAME *game);

#endif //PROJEKT_GAME_H
