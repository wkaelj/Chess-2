#pragma once

// suplimentary rendering functions, such as buttons and chess board

#ifndef bool
#include <stdbool.h>
#endif

#include "../move.h"

#include "render_backend.h"

typedef struct Board Board;
typedef struct Button Button;

typedef struct BoardRect
{
    uint16_t x, y, w, h;
} BoardRect;

typedef struct BoardColour
{
    uint8_t r : 8, g : 8, b : 8, a : 8;
} BoardColour;

Board *create_board(
    const Render *render,
    const char *boardTexture,
    const char *pieceTexture,
    const char *hoverTexture,
    const char *legalMoveTexture,
    const char *fontPath,
    bool playerIsWhite);
void destroy_board(Board *r);

Button *create_button(
    const Render *render,
    const RenderFont *font,
    const char *text,
    const RenderRect *position,
    const BoardColour background,
    const BoardColour border,
    const uint8_t border_width,
    const uint8_t padding);

bool button_clicked(const Render *render, Button *button);
void button_draw(const Render *R, Button *button);

void destroy_button(Button *button);
void button_set_pos(Button *button, uint16_t x, uint16_t y);

void board_update(const Render *render, Board *board, RenderEvent e);

void board_draw(
    const Render *render,
    Board *b,
    const ChessBoard *chessBoard,
    const ChessMoveList *list);