#include "game.h"

#include "render/render.h"
#include "move.h"
#include "render/render_backend.h"
#include <assert.h>
#include <stdio.h>

struct Game
{
    Render *render;
    RenderWindow *window;
    Board *boardRender;
    Button *playButton;
    Button *replayButton;
    Button *exitButton;

    RenderFont *defaultFont;

    ChessBoard *chessBoard;

    GameState state;

    bool quit;
};

const char *BOARD_TEXTURE      = "textures/board.png";
const char *PIECE_TEXTURE      = "textures/pieces_high_res.png";
const char *HOVER_TEXTURE      = "textures/hover.png";
const char *LEGAL_MOVE_TEXTURE = "textures/legal_move.png";

const char *FONT_PATH = "fonts/Nunito-Regular.ttf";

Game *game_init(void)
{

    if (render_init() == RENDER_FAILURE)
        return NULL;
    assert(render_is_initialized() == true);
    Game *g   = malloc(sizeof(Game));
    g->window = render_create_window("Chess", 800, 600);
    assert(g->window);
    g->render = render_create_render(g->window);
    assert(g->render);
    g->defaultFont = render_create_font(g->render, FONT_PATH, 255);

    const BoardColour background = {124, 142, 179, 255};
    const BoardColour border     = {176, 202, 255, 255};
    const int padding            = 50;
    const int borderWidth        = 4;
    int window_w, window_h;
    render_get_window_size(g->window, &window_w, &window_h);

    RenderRect playButtonPos = {
        .x = window_w / 2,
        .y = window_h / 2,
        .w = 200,
        .h = 0,
    };
    g->playButton = create_button(
        g->render,
        g->defaultFont,
        "Play Chess",
        &playButtonPos,
        background,
        border,
        borderWidth,
        padding);

    RenderRect replayButtonPos = {
        .x = window_w / 2,
        .y = window_h / 3,
        .w = 200,
        .h = 0,
    };
    g->replayButton = create_button(
        g->render,
        g->defaultFont,
        "Play Again?",
        &replayButtonPos,
        background,
        border,
        borderWidth,
        padding);

    RenderRect quitButtonPos = {
        .x = window_w / 2,
        .y = window_h * 2 / 3,
        .w = 200,
        .h = 0,
    };
    g->exitButton = create_button(
        g->render,
        g->defaultFont,
        "Quit Chess?",
        &quitButtonPos,
        background,
        border,
        borderWidth,
        padding);

    g->boardRender = create_board(
        g->render,
        BOARD_TEXTURE,
        PIECE_TEXTURE,
        HOVER_TEXTURE,
        LEGAL_MOVE_TEXTURE,
        FONT_PATH,
        true);

    g->state = GAME_STATE_STARTING;
    g->quit  = false;

    g->chessBoard = chess_board_init();

    return g;
}

void game_destroy(Game *g)
{
    chess_board_destroy(g->chessBoard);
    destroy_board(g->boardRender);
    destroy_button(g->playButton);
    destroy_button(g->replayButton);
    destroy_button(g->exitButton);
    render_destroy_render(g->render);
    render_destroy_window(g->window);

    free(g);

    render_quit();
    assert(render_is_initialized() == false);
}

void game_update(Game *g)
{

    RenderEvent event = render_poll_events(g->render);
    int w, h;
    switch (event)
    {
    case RENDER_EVENT_QUIT: g->quit = true; break;
    case RENDER_EVENT_WINDOW_RESIZE:
        render_get_window_size(g->window, &w, &h);
        button_set_pos(g->playButton, w / 2, h / 2);
        button_set_pos(g->replayButton, w / 2, h * 2 / 3);
        button_set_pos(g->exitButton, w / 2, h / 3);

        break;
    default: break;
    }

    render_clear(g->render);

    ChessMoveList moveList; // must declare outside switch
    switch (g->state)
    {
    case GAME_STATE_STARTING:
        if (button_clicked(g->render, g->playButton))
            g->state = GAME_STATE_RUNNING;
        else
        {
            button_draw(g->render, g->playButton);
            break;
        }
    case GAME_STATE_RUNNING:
        chess_board_gen_movelist(g->chessBoard, &moveList);

        board_update(g->render, g->boardRender, event);

        board_draw(g->render, g->boardRender, g->chessBoard, &moveList);

        if (chess_board_get_game_end(g->chessBoard) != GAME_NOT_ENDED)
        {
            g->state = GAME_STATE_ENDED;
        }
        else
            break;
    case GAME_STATE_ENDED:
        if (button_clicked(g->render, g->replayButton))
        {
            chess_board_destroy(g->chessBoard);
            g->chessBoard = chess_board_init();
            g->state      = GAME_STATE_RUNNING;
        }
        else if (button_clicked(g->render, g->exitButton))
        {
            g->quit = true;
        }
        else
        {
            button_draw(g->render, g->replayButton);
            button_draw(g->render, g->exitButton);
        }
        break;
    }
    const uint8_t background = 0x0f;
    render_set_colour(g->render, background, background, background, 0xff);
    render_submit(g->render);
}

bool game_should_quit(const Game *g) { return g->quit; }
