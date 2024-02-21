#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    RENDER_SUCCESS = 0,
    RENDER_FAILURE = 1,
} RenderResult;

typedef enum
{
    RENDER_CURSOR_PRESSED,
    RENDER_CURSOR_RELEASED,
    RENDER_CURSOR_DOWN,
    RENDER_CURSOR_UP,
} RenderCursorState;

typedef enum
{
    RENDER_EVENT_NONE = 0,
    RENDER_EVENT_QUIT,
    RENDER_EVENT_WINDOW_RESIZE,
} RenderEvent;

// created in a window, can draw textures and boxes
typedef struct Render Render;

// a window that can be used to create a render
typedef struct RenderWindow RenderWindow;

// a texture loaded from a image file that can be drawn to the screen
typedef struct RenderTexture RenderTexture;

// a font loaded from a .ttf file used to create text
typedef struct RenderFont RenderFont;

// text created from a font that can be drawn to the screen
typedef struct RenderText RenderText;

// a x, y, width and height specify an area on the screen or a size
typedef struct
{
    int x, y, w, h;
} RenderRect;

typedef struct
{
    int x, y;
} RenderCoord;

// initialize the render backend
RenderResult render_init();

// quit the render backend and release resources
void render_quit();

// check if the render is initialized
bool render_is_initialized();

// create a window. NULL for failure
RenderWindow *render_create_window(const char *title, int w, int h);

// create a render. NULL for failure
Render *render_create_render(RenderWindow *window);

void render_destroy_render(Render *render);
void render_destroy_window(RenderWindow *window);

// get the size of a window. the x, y properties are not used
RenderResult render_get_window_size(const RenderWindow *window, int *w, int *h);

RenderResult render_get_render_size(const Render *render, int *w, int *h);

// draw a texture to the screen
RenderResult render_draw_texture(
    const Render *render,
    const RenderRect *dst_rect,
    const RenderRect *src_rect,
    const RenderTexture *texture,
    float angle);

RenderResult render_draw_rect(const Render *render, const RenderRect *rect);
RenderResult
render_draw_rects(const Render *render, const RenderRect *rects, size_t n);
RenderResult render_draw_text(
    const Render *r, const RenderText *text, const RenderRect *rect);
RenderResult render_set_colour(
    const Render *render, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// clear the screen of any drawings. should be called at the beginning of a
// frame to avoid the last frame staying in undrawn areas
RenderResult render_clear(const Render *render);

// submit render commands to be drawn. nothing is shown until this is called
void render_submit(const Render *render);

// should be called every frame to read mouse input and check for window close
RenderEvent render_poll_events(Render *render);

RenderTexture *
render_create_texture(const Render *render, const char *texture_path);
void render_destroy_texture(RenderTexture *texture);
RenderResult
render_set_texture_alpha(const RenderTexture *texture, uint8_t alpha);
RenderResult render_get_texture_size(const RenderTexture *t, int *w, int *h);

// font_path is a path to .ttf font
RenderFont *
render_create_font(const Render *render, const char *font_path, uint8_t size);
void render_destroy_font(RenderFont *font);

RenderText *render_create_text(
    const Render *render,
    const RenderFont *font,
    const char *text,
    uint8_t r,
    uint8_t g,
    uint8_t b);

void render_destroy_text(RenderText *text);

float render_text_get_aspect_ratio(const RenderText *text);

RenderResult render_get_cursor_pos(const Render *render, int *x, int *y);
RenderCursorState render_get_cursor_state(const Render *render);
