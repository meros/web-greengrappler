#pragma once
#include "vec2.h"
#include <cmath>
#include <cstdint>

inline float lerp(float min, float max, float ratio) {
    if (ratio < 0.0f) return min;
    if (ratio > 1.0f) return max;
    return min + (max - min) * ratio;
}

inline int lerpInt(float min, float max, float ratio) {
    return static_cast<int>(std::floor(lerp(min, max, ratio)));
}

inline Vec2 sincos(float d) {
    return {std::cos(d), std::sin(d)};
}

inline uint32_t makeRGB(uint8_t r, uint8_t g, uint8_t b) {
    return (0xFFu << 24) | (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}
