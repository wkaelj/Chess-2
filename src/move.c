#include "move.h"

struct ChessBoard
{
    thc_board *thc_b;
    // last move?
};

typedef thc_square ChessSquare;
typedef thc_move ChessMove;

typedef thc_movelist ChessMoveList;

// initialize a chess board
ChessBoard *chess_board_init()
{
    ChessBoard *b = malloc(sizeof(ChessBoard));
    b->thc_b      = thc_board_init();
    return b;
}

// destroy a board created using board_init
void chess_board_destroy(ChessBoard *b)
{
    thc_board_destroy(b->thc_b);
    free(b);
}

// generate a list of legal moves
// each move contains a src tile and a dst tile
void chess_board_gen_movelist(const ChessBoard *b, ChessMoveList *list)
{
    thc_board_gen_legal_move_list(b->thc_b, list);
}

// make a move
// the move should generated by board_gen_legal_movelist
void chess_board_move(const ChessBoard *b, ChessMove m)
{
    thc_board_play_move(b->thc_b, m);
}

bool chess_board_white_to_play(const ChessBoard *b)
{
    return thc_board_is_white_move(b->thc_b);
}

// get a string describing the board
// it uses the standard chars for pieces,
// and spaces for empty tiles
const char *chess_board_get_squares(const ChessBoard *b)
{
    return thc_board_get_squares(b->thc_b);
}

ChessGameEnds chess_board_get_game_end(const ChessBoard *b)
{
    return thc_board_get_game_end(b->thc_b);
}
