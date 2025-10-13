//
// Created by lukas on 10/12/25.
//

#ifndef PROJEKT_GAME_H
#define PROJEKT_GAME_H
#include <stddef.h>

#include "tilemap.h"
#include "game_object.h"

#define STARTING_COUNT_OF_GAME_OBJECTS 32
#define STARTING_AMOUNT_OF_MONEY 1000
#define STARTING_AMOUNT_OF_LIVES 100

// Enemy default data
#define ENEMY_DEFAULT_TIMER 1.f

typedef enum {
    GAME_STATE_START,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER
} GAME_STATE;

typedef struct {
    Texture2D towers;
    Texture2D enemies;
    Texture2D start_screen;
    Texture2D defeat_screen;
} ASSETS;

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

    GAME_STATE state;
    int enemies_defeated;  // Track score
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

#endif //PROJEKT_GAME_H
