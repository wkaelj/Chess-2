
#pragma once

#include "render/render.h"

typedef enum GameState
{
    GAME_STATE_STARTING, // game is not started yet
    GAME_STATE_RUNNING,  // game is in progress
    GAME_STATE_ENDED,    // game has finished
} GameState;

typedef struct Game Game;

Game *game_init(void);

void game_destroy(Game *);

void game_update(Game *);

bool game_should_quit(const Game *);
