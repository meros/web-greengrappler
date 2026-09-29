#pragma once

static constexpr int TICKS_PER_SECOND = 60;
static constexpr int SCREEN_WIDTH = 320;
static constexpr int SCREEN_HEIGHT = 240;
static constexpr int TILE_SIZE = 10;

enum class Button {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FIRE,
    JUMP,
    EXIT,
    FORCE_QUIT,
    COUNT
};

#ifdef HW_RVL
static constexpr int OVERSCAN_X = 16;
static constexpr int OVERSCAN_Y = 12;
static constexpr int VIEWPORT_W = 640 - 2 * OVERSCAN_X;  // 608
static constexpr int VIEWPORT_H = 480 - 2 * OVERSCAN_Y;  // 456
#endif

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};
