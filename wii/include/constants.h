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

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};
