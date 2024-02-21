#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef bool
#include <stdbool.h>
#endif

// basic types
// clang-format off
typedef enum thc_square
{
    a8 = 0,
        b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
    SQUARE_INVALID
} thc_square;
// clang-format on

typedef enum thc_special
{
    NOT_SPECIAL = 0,
    SPECIAL_KING_MOVE, // special only because it changes wking_square,
                       // bking_square
    SPECIAL_WK_CASTLING,
    SPECIAL_BK_CASTLING,
    SPECIAL_WQ_CASTLING,
    SPECIAL_BQ_CASTLING,
    SPECIAL_PROMOTION_QUEEN,
    SPECIAL_PROMOTION_ROOK,
    SPECIAL_PROMOTION_BISHOP,
    SPECIAL_PROMOTION_KNIGHT,
    SPECIAL_WPAWN_2SQUARES,
    SPECIAL_BPAWN_2SQUARES,
    SPECIAL_WEN_PASSANT,
    SPECIAL_BEN_PASSANT,
} thc_special;

typedef enum thc_game_ends
{
    GAME_NOT_ENDED      = 0,
    GAME_END_WCHECKMATE = 1,
    GAME_END_BCHECKMATE = -1,
    GAME_END_STALEMATE  = 2,
    GAME_END_INSUFFICIENT,
    GAME_END_REPITITION,
    GAME_END_50_MOVE,
} thc_game_ends;

// thc::thc_square utilities
static inline uint get_file(thc_square sq)
{
    return (int)(sq)&0x07;
} // eg c5->'c'
static inline uint get_rank(thc_square sq)
{
    return ((int)(sq) >> 3) & 0x07;
} // eg c5->'5'
static inline thc_square make_square(uint file, uint rank)
{
    return (thc_square)(rank * 8 + file);
} // eg ('c','5') -> c5
// each type is declared with the functions below it

// the move type

#define MAXMOVES (27 + 2 * 13 + 2 * 14 + 2 * 8 + 8 + 8 * 4 + 3 * 27)
typedef struct thc_move
{
    thc_square src : 8;
    thc_square dst : 8;
    thc_special special : 8;
    uint8_t capture : 8;
} thc_move;

typedef struct thc_movelist
{
    int count : 32;
    thc_move moves[MAXMOVES];
} thc_movelist;

typedef struct thc_board thc_board;

thc_board *thc_board_init();
void thc_board_destroy(thc_board *);
void thc_board_gen_legal_move_list(thc_board *, thc_movelist *);
void thc_board_play_move(thc_board *, thc_move);
bool thc_board_is_white_move(thc_board *);
char *thc_board_get_squares(thc_board *);          // do not edit the string
thc_game_ends thc_board_get_game_end(thc_board *); // GAME_NOT_ENDED, or end
