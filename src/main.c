
#include <stdio.h>

#include "game.h"

int main(int argc, char **argv)
{
    Game *g = game_init();
    while (game_should_quit(g) != true)
        game_update(g);
    game_destroy(g);
    return 0;
}