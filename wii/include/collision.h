#pragma once

struct Collidable {
    virtual float getCollideTop() const = 0;
    virtual float getCollideLeft() const = 0;
    virtual float getCollideBottom() const = 0;
    virtual float getCollideRight() const = 0;
    virtual ~Collidable() = default;
};

inline bool collides(const Collidable& a, const Collidable& b) {
    if (a.getCollideRight() < b.getCollideLeft()) return false;
    if (a.getCollideLeft() > b.getCollideRight()) return false;
    if (a.getCollideBottom() < b.getCollideTop()) return false;
    if (a.getCollideTop() > b.getCollideBottom()) return false;
    return true;
}

inline bool collidesRect(const Collidable& a, float left, float top, float right, float bottom) {
    if (a.getCollideRight() < left) return false;
    if (a.getCollideLeft() > right) return false;
    if (a.getCollideBottom() < top) return false;
    if (a.getCollideTop() > bottom) return false;
    return true;
}

struct SimpleCollidable : public Collidable {
    float left, top, right, bottom;
    SimpleCollidable(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
    float getCollideTop() const override { return top; }
    float getCollideLeft() const override { return left; }
    float getCollideBottom() const override { return bottom; }
    float getCollideRight() const override { return right; }
};
