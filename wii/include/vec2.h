#pragma once
#include <cmath>

static constexpr float VEC2_EPSILON = 0.0000001f;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float f) const { return {x * f, y * f}; }
    Vec2 operator/(float d) const { return {x / d, y / d}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float f) { x *= f; y *= f; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSq() const { return x * x + y * y; }
    bool isZero() const { return length() < VEC2_EPSILON; }
    float dot(const Vec2& o) const { return x * o.x + y * o.y; }

    float lengthCompare(float val) const { return lengthSq() - val * val; }
    float lengthCompare(const Vec2& o) const { return lengthSq() - o.lengthSq(); }

    Vec2 normalized() const {
        float len = length();
        if (len < VEC2_EPSILON) return *this;
        return *this / len;
    }

    Vec2& normalize() {
        float len = length();
        if (len < VEC2_EPSILON) return *this;
        x /= len; y /= len;
        return *this;
    }
};
