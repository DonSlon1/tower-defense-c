#ifndef MULTIPLAYER_GAME_H
#define MULTIPLAYER_GAME_H

#include "game.h"
#include "network.h"

void run_multiplayer_host_game(network_state* net, int window_width, int window_height);
void run_multiplayer_client_game(network_state* net, int window_width, int window_height);

#endif
