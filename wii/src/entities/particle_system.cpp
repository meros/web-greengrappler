#include "entities/particle_system.h"
#include "media/animation.h"
#include "constants.h"
#include "utils.h"
#include <cmath>
#include <cstdlib>

ParticleSystem::ParticleSystem(Animation* animation, int animSpeed, int lifeTime, int blinkTime,
                               float minSpeed, float maxSpeed, int numParticles,
                               Vec2 initialVel, float gravity)
    : animation_(animation)
    , animationSpeed_(animSpeed)
    , blinkTimeTicks_(blinkTime)
    , gravity_(Vec2(0, gravity / TICKS_PER_SECOND))
    , initialVel_(initialVel)
    , maxSpeed_(maxSpeed)
    , minSpeed_(minSpeed)
    , numParticles_(numParticles)
    , lifeTimeTicks_(lifeTime) {
    Entity::setSize(Vec2(30, 30));
}

void ParticleSystem::onRespawn() {
    remove();
}

void ParticleSystem::setPosition(Vec2 pos) {
    setPositionWithSpread(pos, 0, false);
}

void ParticleSystem::setPositionWithSpread(Vec2 position, float randomAmount, bool randomVel) {
    Entity::setPosition(position);

    for (int i = 0; i < numParticles_; i++) {
        Vec2 pos = position_;
        pos.x += (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * randomAmount;
        pos.y += (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * randomAmount;
        particlesPos_.push_back(pos);

        float r0710 = static_cast<float>(rand()) / RAND_MAX * 0.3f + 0.7f;
        Vec2 velocity;
        if (randomVel) {
            velocity = Vec2(r0710, r0710);
        } else {
            velocity = sincos(2.0f * static_cast<float>(M_PI) * static_cast<float>(rand()) / RAND_MAX);
        }

        float factor = static_cast<float>(rand()) / RAND_MAX * (maxSpeed_ - minSpeed_) + minSpeed_;
        velocity *= factor;
        velocity += initialVel_;
        velocity *= (1.0f / TICKS_PER_SECOND);

        particlesVel_.push_back(velocity);
    }
}

void ParticleSystem::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    if (lifeTimeTicks_ < blinkTimeTicks_ && lifeTimeTicks_ % 2 == 0) return;

    int seed = 0;
    auto seededRandom = [&seed]() -> int {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        return seed;
    };

    for (size_t i = 0; i < particlesPos_.size(); i++) {
        const Vec2& p = particlesPos_[i];
        int frameIdx = std::abs(frameNum_ / animationSpeed_ + seededRandom());
        int x = static_cast<int>(std::round(p.x + offsetX)) - animation_->getFrameWidth() / 2;
        int y = static_cast<int>(std::round(p.y + offsetY)) - animation_->getFrameHeight() / 2;
        animation_->drawFrame(renderer, frameIdx, x, y);
    }
}

void ParticleSystem::update() {
    frameNum_++;
    if (lifeTimeTicks_ > 0) {
        for (size_t i = 0; i < particlesVel_.size(); i++) {
            particlesPos_[i] += particlesVel_[i];
            particlesVel_[i] += gravity_;
        }
        lifeTimeTicks_--;
    } else {
        remove();
    }
}
