#include "thc.h"
#include <cstdint>
#include <stdlib.h>

struct thc_move
{
    thc::Square src : 8;
    thc::Square dst : 8;
    thc::SPECIAL special : 8;
    uint8_t capture : 8;
};

struct thc_board
{
    thc::ChessRules internal_board;
};

struct thc_movelist
{
    int count : 32;
    thc_move moves[MAXMOVES];
};

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

// extern "C" thc_move thc_move_init();
extern "C" thc_board *thc_board_init();
extern "C" void thc_board_destroy(thc_board *);
extern "C" void thc_board_gen_legal_move_list(thc_board *, thc_movelist *);
extern "C" void thc_board_play_move(thc_board *, thc_move);
extern "C" bool thc_board_is_white_move(thc_board *);
extern "C" char *thc_board_get_squares(thc_board *); // do not edit the string
extern "C" thc_game_ends thc_board_get_game_end(thc_board *);
// thc move helper
thc::Move cast_to_thc_move(thc_move m)
{
    thc::Move thc;
    thc.src     = m.src;
    thc.dst     = m.dst;
    thc.special = m.special;
    thc.capture = m.capture;
    return thc;
}

thc_move cast_from_thc_move(thc::Move thc)
{
    return (thc_move){
        .src     = thc.src,
        .dst     = thc.dst,
        .special = thc.special,
        .capture = (unsigned char)thc.capture,
    };
}

thc_board *thc_board_init()
{
    thc_board *b = (thc_board *)malloc(sizeof(struct thc_board));
    thc::ChessRules rules{};
    rules.Init();
    *b = (thc_board){rules};

    return b;
}

void thc_board_destroy(thc_board *b) { free(b); }

#include <cassert>

void thc_board_gen_legal_move_list(thc_board *b, thc_movelist *list)
{

    thc::MOVELIST thc_list;
    b->internal_board.GenLegalMoveList(&thc_list);
    list->count = thc_list.count;
    for (size_t i = 0; i < thc_list.count; i++)
        list->moves[i] = cast_from_thc_move(thc_list.moves[i]);

    for (size_t i = 0; i < thc_list.count; i++)
        assert(thc_list.moves[i].src == list->moves[i].src);
}

void thc_board_play_move(thc_board *b, thc_move m)
{
    b->internal_board.PlayMove(cast_to_thc_move(m));
}

bool thc_board_is_white_move(thc_board *b)
{
    return b->internal_board.WhiteToPlay();
}

char *thc_board_get_squares(thc_board *b) { return b->internal_board.squares; }

thc_game_ends thc_board_get_game_end(thc_board *b)
{

    thc::TERMINAL terminal;
    b->internal_board.Evaluate(terminal);
    switch (terminal)
    {
    case thc::NOT_TERMINAL: return GAME_NOT_ENDED;
    case thc::TERMINAL_WCHECKMATE: return GAME_END_WCHECKMATE; break;
    case thc::TERMINAL_BCHECKMATE: return GAME_END_BCHECKMATE; break;
    case thc::TERMINAL_BSTALEMATE: return GAME_END_STALEMATE; break;
    case thc::TERMINAL_WSTALEMATE: return GAME_END_STALEMATE; break;
    }

    bool white_asks = true;
    thc::DRAWTYPE draw_type;
    if (b->internal_board.IsDraw(white_asks, draw_type))
    {
        switch (draw_type)
        {
        case thc::NOT_DRAW: return GAME_NOT_ENDED;
        case thc::DRAWTYPE_REPITITION: return GAME_END_REPITITION;
        case thc::DRAWTYPE_50MOVE: return GAME_END_50_MOVE;
        case thc::DRAWTYPE_INSUFFICIENT: return GAME_END_INSUFFICIENT;
        case thc::DRAWTYPE_INSUFFICIENT_AUTO: return GAME_END_INSUFFICIENT;
        }
    }

    return GAME_NOT_ENDED;
}