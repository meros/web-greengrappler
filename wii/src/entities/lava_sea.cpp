#include "entities/lava_sea.h"
#include "entities/particle_system.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"
#include "camera.h"
#include "constants.h"
#include <cmath>
#include <cstdlib>

LavaSea::LavaSea() {
    topAnim_ = Resource::getAnimation("data/images/lavatop.bmp", 2);
    fillAnim_ = Resource::getAnimation("data/images/lavafill.bmp", 2);
    setSize(Vec2(10, 10));
}

float LavaSea::getCurrentY() const {
    if (safeMode_) return safeLevel_;
    float sinwave = (std::sin(frame_ / 100.0f) + 1.0f) / 2.0f;
    sinwave = sinwave * sinwave * sinwave;
    return std::round(position_.y - sinwave * 80.0f + 2.0f);
}

void LavaSea::onLevelComplete() {
    safeLevel_ = getCurrentY();
    safeMode_ = true;
}

void LavaSea::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int screenY = offsetY + static_cast<int>(getCurrentY());
    int animFrame = (frame_ / 30);

    for (int x = -10 + offsetX % 10; x < SCREEN_WIDTH; x += 10) {
        topAnim_->drawFrame(renderer, animFrame, x, screenY);
    }
    for (int x = -10 + offsetX % 10; x < SCREEN_WIDTH; x += 10) {
        for (int y = screenY + 10; y < SCREEN_HEIGHT; y += 10) {
            fillAnim_->drawFrame(renderer, animFrame, x, y);
        }
    }
}

void LavaSea::update() {
    if (room_->getHero()->Collides(*this)) {
        room_->getHero()->kill();
    }

    frame_++;

    if (safeMode_) {
        safeLevel_ += 1.0f;
        if (safeLevel_ > position_.y + 10.0f) {
            safeLevel_ = std::round(position_.y + 10.0f);
        }
    }

    if (static_cast<float>(rand()) / RAND_MAX < 0.3f) {
        ParticleSystem* ps = new ParticleSystem(
            Resource::getAnimation("data/images/particles.bmp"),
            2, 30, 10, 1.0f, 50.0f, 10, Vec2(0, -30), 5.0f);
        float minX = static_cast<float>(-room_->getCamera()->getOffsetX());
        float maxX = static_cast<float>(SCREEN_WIDTH - room_->getCamera()->getOffsetX());
        float rVal = static_cast<float>(std::floor(
            static_cast<float>(rand()) / RAND_MAX * (maxX - minX) + minX));
        ps->setPositionWithSpread(Vec2(rVal, getCurrentY()), 5.0f, false);
        room_->addEntity(ps);
    }
}
