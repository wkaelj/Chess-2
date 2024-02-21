#include "render.h"

#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "render_backend.h"
#include <malloc.h>

#define alloc(type) (malloc(sizeof(type)))

#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define MOUSE_AVERAGE_SIZE 50

struct Board
{
    bool playerIsWhite;

    size_t mouseAverageIndex;
    int mouseAverage[MOUSE_AVERAGE_SIZE]; // average position of the mouse x

    RenderCursorState rmb, lmb; // true if button pressed last frame

    ChessSquare hoveredTile : 8; // the tile the hovered piece was on
    char hoveredPiece;

    bool shouldQuit;
    ChessMoveList moveList;

    RenderRect boardRect;

    RenderFont *defaultFont;

    // textures
    struct
    {
        RenderTexture *board;
        RenderTexture *pieces;
        RenderTexture *hover;
        RenderTexture *legalMove;
    } textures;
};

struct Button
{
    uint8_t padding, borderWidth;
    BoardColour borderColour, backgroundColour;
    RenderText *buttonText;
    RenderRect position;
};

// drawing helpers
void drawBoard(
    const Render *r,
    Board *b,
    const ChessBoard *chessBoard,
    const RenderRect *rect,
    const ChessMoveList *list,
    bool playerIsWhite);
RenderRect getPieceSrcRect(Board *r, char p);
RenderRect getPieceDestRect(const RenderRect *boardRect, ChessSquare index);
// calculate the rect for the largest square that could fit in a rectangle
RenderRect calculateRenderRect(const RenderRect *frame);
ChessSquare getMouseTile(const Render *render, const RenderRect *boardRect);
RenderRect
calculateRectCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

Board *create_board(
    const Render *render,
    const char *boardTexture,
    const char *pieceTexture,
    const char *hoverTexture,
    const char *legalMoveTexture,
    const char *fontPath,
    bool playerIsWhite)
{
    assert(boardTexture);
    assert(pieceTexture);
    assert(render_is_initialized());

    Board *b             = alloc(Board);
    b->hoveredTile       = SQUARE_INVALID;
    b->hoveredPiece      = ' ';
    b->shouldQuit        = false;
    b->rmb               = RENDER_CURSOR_UP;
    b->lmb               = RENDER_CURSOR_UP;
    b->playerIsWhite     = playerIsWhite;
    b->mouseAverageIndex = 0;

    // load piece textures
    b->textures.board     = render_create_texture(render, boardTexture);
    b->textures.pieces    = render_create_texture(render, pieceTexture);
    b->textures.hover     = render_create_texture(render, hoverTexture);
    b->textures.legalMove = render_create_texture(render, legalMoveTexture);
    assert(
        b->textures.board && b->textures.pieces && b->textures.hover &&
        b->textures.legalMove && "All textures must have sucessfully loaded");
    render_set_texture_alpha(b->textures.legalMove, 0x80);
    assert(b->textures.board && b->textures.pieces);

    b->defaultFont = render_create_font(render, fontPath, UINT8_MAX);
    assert(b->defaultFont);

    RenderRect windowRect = {.x = 0, .y = 0};
    render_get_render_size(render, &windowRect.w, &windowRect.h);

    b->boardRect = calculateRenderRect(&windowRect);

    return b;
}

void destroy_board(Board *r)
{
    render_destroy_font(r->defaultFont);

    render_destroy_texture(r->textures.board);
    render_destroy_texture(r->textures.hover);
    render_destroy_texture(r->textures.legalMove);
    render_destroy_texture(r->textures.pieces);

    free(r);
}

Button *create_button(
    const Render *render,
    const RenderFont *font,
    const char *text,
    const RenderRect *position,
    const BoardColour background,
    const BoardColour border,
    const uint8_t border_width,
    const uint8_t padding)
{
    Button *button = alloc(Button);

    RenderText *textObj = render_create_text(render, font, text, 255, 255, 255);

    *button = (Button){
        .padding          = padding,
        .borderWidth      = border_width,
        .borderColour     = border,
        .backgroundColour = background,
        .position         = *position,
        .buttonText       = textObj,
    };

    // change height
    button->position.h =
        button->position.w / render_text_get_aspect_ratio(button->buttonText);

    return button;
}

bool buttonHovered(const Render *render, Button *button)
{
    int mouseX, mouseY;
    render_get_cursor_pos(render, &mouseX, &mouseY);

    const RenderRect *rect = &button->position;
    RenderRect buttonShape = calculateRectCentered(
        rect->x,
        rect->y,
        rect->w + button->padding * 2,
        rect->h + button->padding * 2);

    bool withinH =
        mouseY > buttonShape.y && mouseY < (buttonShape.y + buttonShape.h);
    bool withinX =
        mouseX > buttonShape.x && mouseX < (buttonShape.x + buttonShape.w);

    RenderCursorState cursorState = render_get_cursor_state(render);

    return withinH && withinX;
}

bool button_clicked(const Render *render, Button *button)
{

    return buttonHovered(render, button) &&
           render_get_cursor_state(render) == RENDER_CURSOR_PRESSED;
}

void button_set_pos(Button *button, uint16_t x, uint16_t y)
{
    button->position.x = x;
    button->position.y = y;
}

void button_draw(const Render *render, Button *button)
{
    RenderRect textRect = calculateRectCentered(
        button->position.x,
        button->position.y,
        button->position.w,
        button->position.h);
    RenderRect outerRect = calculateRectCentered(
        button->position.x,
        button->position.y,
        button->position.w + button->padding * 2,
        button->position.h + button->padding * 2);

    // draw border
    RenderRect borderRect = calculateRectCentered(
        button->position.x,
        button->position.y,
        outerRect.w + button->borderWidth * 2,
        outerRect.h + button->borderWidth * 2);
    render_set_colour(
        render,
        button->borderColour.r,
        button->borderColour.g,
        button->borderColour.b,
        button->borderColour.a);
    render_draw_rect(render, &borderRect);

    // draw background
    bool hovered = buttonHovered(render, button);
    int dif      = -20;
    render_set_colour(
        render,
        hovered ? button->backgroundColour.r + dif : button->backgroundColour.r,
        hovered ? button->backgroundColour.g + dif : button->backgroundColour.g,
        hovered ? button->backgroundColour.b + dif : button->backgroundColour.b,
        hovered ? button->backgroundColour.a + dif
                : button->backgroundColour.a);
    render_draw_rect(render, &outerRect);

    // draw text
    render_draw_text(render, button->buttonText, &textRect);
}

void destroy_button(Button *b)
{
    render_destroy_text(b->buttonText);
    free(b);
}

void board_update(const Render *render, Board *r, RenderEvent e)
{

    RenderRect windowRect = {};

    switch (e)
    {
    case RENDER_EVENT_NONE: break;
    case RENDER_EVENT_QUIT: r->shouldQuit = true; break;
    case RENDER_EVENT_WINDOW_RESIZE:
        render_get_render_size(render, &windowRect.w, &windowRect.h);

        r->boardRect = calculateRenderRect(&windowRect);
        break;
    default: break;
    }

    r->lmb = render_get_cursor_state(render);
}

void board_draw(
    const Render *render,
    Board *board,
    const ChessBoard *chessBoard,
    const ChessMoveList *list)
{
    assert(board);

    int window_w, window_h;
    render_get_render_size(render, &window_w, &window_h);

    size_t padding = 10;

    // padding must be even!!!
    if (padding % 2 != 0)
        padding++;

    RenderRect board1 = {
        .x = padding / 2,
        .y = padding / 2,
        .w = window_w - padding,
        .h = window_h - padding,
    };

    // this shouldn't break, because board1 shouldn't be changed until after the
    // function returns
    board1 = calculateRenderRect(&board1);

    drawBoard(render, board, chessBoard, &board1, list, board->playerIsWhite);
}

void drawBoard(
    const Render *render,
    Board *b,
    const ChessBoard *chessBoard,
    const RenderRect *boardRect,
    const ChessMoveList *list,
    bool playerIsWhite)
{
    // draw board
    render_draw_texture(render, boardRect, NULL, b->textures.board, 0.f);

    int mouse_x, mouse_y;
    render_get_cursor_pos(render, &mouse_x, &mouse_y);

    // highlight lastmove

    // highlight mouse hover
    ChessSquare mouseTile  = getMouseTile(render, boardRect);
    uint8_t mousePieceTile = !playerIsWhite ? 63 - mouseTile : mouseTile;
    if (mouseTile < 64)
    {
        RenderRect mouseRect = getPieceDestRect(boardRect, mouseTile);
        render_draw_texture(render, &mouseRect, NULL, b->textures.hover, 0.f);
    }

    // get board string
    const char *squares = chess_board_get_squares(chessBoard);

    // draw pieces
    for (size_t i = 0; squares[i] != 0; i++)
    {
        if (squares[i] != ' ')
        {
            RenderRect destRect = getPieceDestRect(boardRect, i);
            RenderRect srcRect  = getPieceSrcRect(b, squares[i]);

            size_t hovertile = b->hoveredTile;
            if (b->hoveredTile != SQUARE_INVALID && i == hovertile)
                render_set_texture_alpha(b->textures.pieces, 0x80);
            render_draw_texture(
                render, &destRect, &srcRect, b->textures.pieces, 0);
            if (b->hoveredTile != SQUARE_INVALID && i == hovertile)
                render_set_texture_alpha(b->textures.pieces, 0xff);
        }
    }

    // get hovered piece
    if (mouseTile < 64 && b->lmb == RENDER_CURSOR_PRESSED &&
        (squares[mouseTile] != ' ' &&
         chess_board_white_to_play(chessBoard) ==
             (tolower(squares[mousePieceTile]) != squares[mousePieceTile])))
    {
        b->hoveredPiece = squares[mousePieceTile];
        b->hoveredTile  = mousePieceTile;
        if (b->hoveredPiece != ' ')
        {
            // reset mouse average
            for (size_t i = 0; i < MOUSE_AVERAGE_SIZE; i++)
                b->mouseAverage[i] = mouse_x;
        }
    }

    // report move attempt
    if (b->lmb == RENDER_CURSOR_RELEASED && mousePieceTile < 64 &&
        b->hoveredPiece != ' ')
    {
        if (mousePieceTile != b->hoveredTile)
        {
            for (size_t i = 0; i < list->count; i++)
            {
                if (list->moves[i].src == b->hoveredTile &&
                    list->moves[i].dst == mousePieceTile)
                    chess_board_move(chessBoard, list->moves[i]);
            }
        }
        b->hoveredPiece = ' ';
        b->hoveredTile  = SQUARE_INVALID;
    }

    for (size_t i = 0; b->hoveredTile != SQUARE_INVALID && i < list->count; i++)
    {
        if (list->moves[i].src == b->hoveredTile)
        {
            const RenderRect tileRect =
                getPieceDestRect(boardRect, list->moves[i].dst);
            render_draw_texture(
                render, &tileRect, NULL, b->textures.legalMove, 0.f);
        }
    }

    // draw hovered piece
    if ((b->hoveredPiece) != ' ' &&
        (b->lmb == RENDER_CURSOR_PRESSED || b->lmb == RENDER_CURSOR_DOWN))
    {
        RenderRect dragPieceRect = {
            .x = mouse_x - boardRect->w / 12,
            .y = mouse_y - boardRect->w / 12,
            .w = boardRect->w / 6,
            .h = boardRect->h / 6,
        };
        // calculate average mouse position
        float mouseAverageTotal = 0xf;
        for (size_t i = 0; i < MOUSE_AVERAGE_SIZE; i++)
            mouseAverageTotal += b->mouseAverage[i];
        mouseAverageTotal /= MOUSE_AVERAGE_SIZE;

        float rotation = (mouse_x - mouseAverageTotal) * 1.f;
        rotation       = (90.f / (M_PI / 2.f)) * atan(rotation / 32.f);

        RenderRect pieceRect = getPieceSrcRect(b, b->hoveredPiece);
        render_set_texture_alpha(b->textures.pieces, 64 * 3);
        render_draw_texture(
            render, &dragPieceRect, &pieceRect, b->textures.pieces, rotation);
        render_set_texture_alpha(b->textures.pieces, UINT8_MAX);

        // slowly move average back to mouse position when mouse is stopped
        if (mouseAverageTotal != mouse_x)
        {
            assert(b->mouseAverageIndex < MOUSE_AVERAGE_SIZE);
            b->mouseAverage[b->mouseAverageIndex] = mouse_x;
            b->mouseAverageIndex =
                (b->mouseAverageIndex + 1) % MOUSE_AVERAGE_SIZE;
        }
    }
}

RenderRect getPieceSrcRect(Board *r, char p)
{
    assert(isalpha(p) || p == ' ');

    int textureSize[2];
    render_get_texture_size(
        r->textures.pieces, &textureSize[0], &textureSize[1]);
    int x           = 0;
    const int sixth = textureSize[0] / 6;
    switch (tolower(p))
    {
    case ' ': x = 0; break;
    case '\0': x = 0; break;
    case 'p': x = 5 * sixth; break;
    case 'n': x = 3 * sixth; break;
    case 'b': x = 2 * sixth; break;
    case 'r': x = 4 * sixth; break;
    case 'q': x = 1 * sixth; break;
    case 'k': x = 0; break;
    default: assert(0 && "Invalid piece should not be used");
    };

    int y = p == tolower(p) ? textureSize[1] / 2 : 0;

    RenderRect srcRect = {
        .x = x,
        .y = y,
        .w = sixth,
        .h = textureSize[1] / 2,
    };

    if (p == ' ')
    {
        srcRect.w = 1;
        srcRect.h = 1;
    }

    return srcRect;
}

RenderRect
getPieceDestRect(const RenderRect *boardRect, const ChessSquare square)
{
    float w = boardRect->w / 8.f;
    float h = boardRect->h / 8.f;

    float x      = get_file(square);
    float y      = get_rank(square);
    float eighth = boardRect->w / 8.f;
    x *= eighth;
    y *= eighth;

    return (RenderRect){
        .x = (int)roundf(x) + boardRect->x,
        .y = (int)roundf(y) + boardRect->y,
        .w = (int)roundf(w),
        .h = (int)roundf(h),
    };
}

RenderRect calculateRenderRect(const RenderRect *frame)
{
    int windowSize[2] = {frame->w, frame->h};

    // draw board
    int min = windowSize[0] < windowSize[1] ? windowSize[0] : windowSize[1];

    RenderRect boardRect;
    if (min == windowSize[0])
    {
        boardRect = (RenderRect){
            .x = 0 + frame->x,
            .y = (windowSize[1] - min) / 2 + frame->y,
            .w = min,
            .h = min,
        };
    }
    else
    {
        boardRect = (RenderRect){
            .x = (windowSize[0] - min) / 2 + frame->x,
            .y = 0 + frame->y,
            .w = min,
            .h = min,
        };
    }

    return boardRect;
}

ChessSquare getMouseTile(const Render *render, const RenderRect *boardRect)
{
    int mouse_x, mouse_y;
    render_get_cursor_pos(render, &mouse_x, &mouse_y);
    assert(boardRect->w == boardRect->h);
    int x = mouse_x, y = mouse_y;

    x -= boardRect->x;
    y -= boardRect->y;
    if (x <= 2 || y <= 2 || x > boardRect->w - 2 || y > boardRect->h - 2)
        return UINT8_MAX;

    unsigned tilesize = boardRect->w / 8;

    y /= tilesize;
    x /= tilesize;

    return make_square(x, y);
}

RenderRect calculateRectCentered(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    return (RenderRect){.x = x - w / 2, .y = y - h / 2, .w = w, .h = h};
}
