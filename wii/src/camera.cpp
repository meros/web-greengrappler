#include "camera.h"
#include "constants.h"
#include "entities/hero.h"
#include <cmath>
#include <cstdlib>

Camera::Camera(Vec2 topLeft, Vec2 bottomRight)
    : topLeft_(topLeft), bottomRight_(bottomRight) {}

void Camera::addRect(float x, float y, float w, float h) {
    rects_.push_back({x, y, w, h});
}

void Camera::addShake(float amount, float time) {
    shakeAmount_ = amount;
    shakeTime_ = time;
}

void Camera::centerToHero(const Hero& hero) {
    offset_.x = -hero.getDrawPositionX() + 160;
    offset_.y = -hero.getDrawPositionY() + static_cast<int>((2 * 240) / 3);
}

int Camera::getOffsetX() const {
    return static_cast<int>(std::round(offset_.x + shakeOffset_.x));
}

int Camera::getOffsetY() const {
    return static_cast<int>(std::round(offset_.y + shakeOffset_.y));
}

void Camera::update(const Hero& hero) {
    bool foundRect = false;
    Vec2 heroPos = hero.getPosition();

    for (auto& rect : rects_) {
        if (rect.x < heroPos.x && rect.y < heroPos.y &&
            rect.x + rect.w > heroPos.x && rect.y + rect.h > heroPos.y) {
            desiredOffset_ = {-rect.x, -rect.y + 10};
            foundRect = true;
        }
    }

    if (!foundRect) {
        desiredOffset_ = {
            -hero.getDrawPositionX() + 160.0f,
            -hero.getDrawPositionY() + static_cast<float>(static_cast<int>((2 * 240) / 3))
        };
    }

    desiredOffset_ -= offset_;
    desiredOffset_ *= 0.1f;
    offset_ += desiredOffset_;

    shakeTime_--;
    if (shakeTime_ > 0) {
        shakeOffset_ = {
            shakeAmount_ * ((std::rand() / static_cast<float>(RAND_MAX)) - 0.5f),
            shakeAmount_ * ((std::rand() / static_cast<float>(RAND_MAX)) - 0.5f)
        };
    }
    if (shakeTime_ < 0) {
        shakeOffset_ = {0, 0};
    }

    if (offset_.x + topLeft_.x > 0)
        offset_.x = -topLeft_.x;
    if (offset_.y + topLeft_.y > 10)
        offset_.y = 10 - topLeft_.y;
    if (offset_.x + bottomRight_.x < 320)
        offset_.x = 320 - bottomRight_.x;
    if (offset_.y + bottomRight_.y < 240)
        offset_.y = 240 - bottomRight_.y;
}

void Camera::onRespawn() {
    shakeTime_ = 0;
    shakeAmount_ = 0;
}
