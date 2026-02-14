#pragma once
#include "vec2.h"
#include <vector>
#include <cstdlib>

class Hero;

struct CameraRect {
    float x, y, w, h;
};

class Camera {
public:
    Camera(Vec2 topLeft, Vec2 bottomRight);

    void addRect(float x, float y, float w, float h);
    void addShake(float amount, float time);
    void centerToHero(const Hero& hero);
    void update(const Hero& hero);
    void onRespawn();

    int getOffsetX() const;
    int getOffsetY() const;

private:
    Vec2 offset_;
    Vec2 shakeOffset_;
    float shakeAmount_ = 0;
    float shakeTime_ = 0;
    Vec2 topLeft_;
    Vec2 bottomRight_;
    std::vector<CameraRect> rects_;
    Vec2 desiredOffset_;
};
