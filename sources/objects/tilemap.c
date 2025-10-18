#include "tilemap.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Layer 1 data - base layer
static const int l_new_layer_1[MAP_HEIGHT][MAP_WIDTH] = {
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,249,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,275,299,299,299,299,299},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,248,299,299,299,299,299},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,274,355,355,355,355,355},
   {299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,299,276,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355},
   {355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355,355}
};

// Layer 3 data - top decorative layer
static const int l_new_layer_2[MAP_HEIGHT][MAP_WIDTH] = {
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,44,45},
   {0,0,0,0,0,0,0,0,0,147,148,0,0,0,0,0,0,0,0,0,0,0,56,57,58},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,69,70,71},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,41,42,0,0,0,0,0,0},
   {0,40,41,42,0,0,0,0,0,0,0,0,0,0,0,0,53,54,55,0,0,0,0,0,0},
   {0,53,54,55,0,0,0,0,0,0,0,0,0,149,150,0,66,67,68,0,0,0,0,0,0},
   {0,66,67,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,43,44,45,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,56,57,58,0,0,14,15,16,0,0,0,0,0,0,0,0,0,0},
   {0,149,150,0,0,0,0,69,70,71,0,0,27,28,29,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,40,41,42,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,53,54,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,66,67,68,0,0,0,0,0,0,0,0,147,148,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,40,41,42,0,0,0,147,148,0,0,0,0,43,44,45,147,148,0,40,41,42,0,0},
   {0,0,53,54,55,0,0,0,0,0,0,0,0,0,56,57,58,0,0,0,53,54,55,0,0},
   {0,0,66,67,68,0,0,0,0,0,0,0,0,0,69,70,71,0,0,0,66,67,68,0,0}
};

tile_map init_tilemap() {
    tile_map map;
    map.tile_size = TILE_SIZE;
    map.map_width = MAP_WIDTH;
    map.map_height = MAP_HEIGHT;

    map.tileset1 = load_texture(ASSETS_PATH "images/83291578-f8ec-4e3f-2f6a-6a248efa5800.png");
    if (map.tileset1.id == 0) {
        fprintf(stderr, "ERROR: Failed to load tileset1 texture\n");
        exit(1);
    }

    map.tileset2 = load_texture(ASSETS_PATH "images/bb5eb52a-6c5d-4e83-72e7-a62c7ac8ea00.png");
    if (map.tileset2.id == 0) {
        fprintf(stderr, "ERROR: Failed to load tileset2 texture\n");
        exit(1);
    }

    memcpy(map.layer1, l_new_layer_1, sizeof(l_new_layer_1));
    memcpy(map.layer2, l_new_layer_2, sizeof(l_new_layer_2));

    return map;
}

static void draw_layer(const tile_map* map, const int layer[MAP_HEIGHT][MAP_WIDTH], const texture_2d tileset) {

    for (int y = 0; y < map->map_height; y++) {
        for (int x = 0; x < map->map_width; x++) {
            int tile_index = layer[y][x];

            if (tile_index == 0) continue;

            tile_index -= 1;

            draw_texture(map,tileset,tile_index,x,y);
        }
    }
}
static int tiles_per_row(const tile_map* map, const texture_2d tileset) {
    if (map->tile_size == 0) {
        return 0;
    }
    return (tileset.width + map->tile_size - 1) / map->tile_size;
}

void draw_texture(const tile_map* map, const texture_2d tileset, const int tile_index, const int x, const int y) {

    const int scaled_tile_size = get_tile_scale(map);

    const int tiles_per_row_v = tiles_per_row(map,tileset);
    if (tiles_per_row_v == 0) {
        fprintf(stderr, "ERROR: Failed to count tiles per row\n");
        return;
    }

    const int src_x = tile_index % tiles_per_row_v * map->tile_size;
    const int src_y = tile_index / tiles_per_row_v * map->tile_size;

    const rectangle source = {
        (float)src_x,
        (float)src_y,
        (float)map->tile_size,
        (float)map->tile_size
    };

    const rectangle dest = {
        (float)(x * scaled_tile_size),
        (float)(y * scaled_tile_size),
        (float) scaled_tile_size,
        (float) scaled_tile_size
    };

    draw_texture_pro(tileset, source, dest, (vector2){0, 0}, 0.0f, white);
}

void draw_tilemap(const tile_map* map) {
    draw_layer(map, map->layer1, map->tileset1);
    draw_layer(map, map->layer2, map->tileset2);
}

void unload_tilemap(const tile_map* map) {
    unload_texture(map->tileset1);
    unload_texture(map->tileset2);
}
int get_tile_scale(const tile_map* map) {
    const int screen_width = get_screen_width();
    const int screen_height = get_screen_height();

    if (map->map_width == 0 || map->map_height == 0) {
        fprintf(stderr, "ERROR: Invalid map dimensions for scaling\n");
        return 16;
    }

    const int scale_x = screen_width / map->map_width;
    const int scale_y = screen_height / map->map_height;
    const int scale = scale_x < scale_y ? scale_x : scale_y;

    if (scale <= 0) {
        return 16;
    }

    return scale;
}
