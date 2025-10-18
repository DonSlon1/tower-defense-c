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
} wave_config;

typedef enum {
    game_state_start,
    game_state_playing,
    game_state_wave_break,
    game_state_game_over
} game_state;

typedef struct {
    texture_2d towers;
    texture_2d mushroom_run;
    texture_2d mushroom_hit;
    texture_2d mushroom_die;
    texture_2d flying_fly;
    texture_2d flying_hit;
    texture_2d flying_die;
    texture_2d iceball;
    texture_2d start_screen;
    texture_2d defeat_screen;
} assets;

typedef struct {
    vector2 position;
    bool occupied;
} tower_spot;

typedef struct game {
    game_object *game_objects;
    tile_map tilemap;
    assets assets;

    size_t object_count;
    size_t object_capacity;

    int player_lives;
    int player_money;
    int next_id;
    float enemy_spawn_timer;

    tower_spot tower_spots[4];

    int current_wave;
    int enemies_spawned_in_wave;
    int enemies_alive;
    float wave_break_timer;

    game_state state;
    int enemies_defeated;
} game;

game init_game();
void add_game_object(game *g, game_object obj);
void grow_object_capacity(game *g);
void start_game(game *g);
void unload_game(game *g);
grid_coord screen_to_grid(vector2 screen_pos, const tile_map* tilemap);
int get_game_objects_of_type(const game *g, object_type type, game_object **out_objects);
void update_game_state(game *g, float delta_time);
void remove_inactive_objects(game *g);
game_object* find_tower_at_grid(const game *g, grid_coord coord);
int find_tower_spot_at_grid(const game *g, grid_coord coord);
bool try_build_tower(game *g, int spot_index);
wave_config get_wave_config(int wave_number);
void start_next_wave(game *g);
void handle_playing_input(game *g);
void spawn_enemy(game *g);
void reset_game(game *g);

#endif //PROJEKT_GAME_H
