#include "entities/wall_of_death.h"
#include "entities/particle_system.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"
#include "player_skill.h"
#include "utils.h"
#include <cmath>

WallOfDeath::WallOfDeath() {
    setSize(Vec2(10, 12 * 100));
    saw_ = Resource::getAnimation("data/images/saw.bmp");
}

void WallOfDeath::onRespawn() {
    room_->destroyToTileRow(-1);
    setPosition(originalPosition_);
    running_ = false;
    boost_ = false;
    soundCountdown_ = 10.0f;
}

void WallOfDeath::onStartWallOfDeath() {
    if (!running_) {
        running_ = true;
    } else {
        boost_ = true;
    }
}

void WallOfDeath::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(getDrawPositionX() + offsetX - size_.x / 2.0f));
    int y = static_cast<int>(std::round(getDrawPositionY() + offsetY - size_.y / 2.0f));
    int saws = static_cast<int>(size_.y) / 12;

    for (int i = 0; i < saws; i++) {
        saw_->drawFrame(renderer, wallFrameCounter_ / 5 + i, x, y + i * 12);
    }
}

void WallOfDeath::update() {
    wallFrameCounter_++;

    if (!inited_) {
        originalPosition_ = position_;
        inited_ = true;
    }

    if (!running_) return;

    if (position_.x > 2950.0f) {
        ParticleSystem* ps = new ParticleSystem(saw_, 2, 100, 20, 10.0f, 20.0f, 1, Vec2(0, -50), 4.0f);
        int x = static_cast<int>(std::round(getDrawPositionX() - size_.x / 2.0f));
        int y = static_cast<int>(std::round(getDrawPositionY() - size_.y / 2.0f));
        int saws = static_cast<int>(size_.y) / 12;

        for (int i = 0; i < saws; i++) {
            ps->setPositionWithSpread(Vec2(static_cast<float>(x), static_cast<float>(y + i * 12)), 0.1f, true);
        }
        room_->addEntity(ps);
        remove();
        return;
    }

    for (int xo = 0; xo < 3; xo++) {
        int xt = xo + static_cast<int>(std::floor((position_.x - size_.x / 2.0f) / room_->getTileWidth()));
        room_->destroyToTileRow(xt - 1);
    }

    float heroX = room_->getHero()->getPosition().x;
    float x = position_.x;
    float distance = heroX - x;

    if (distance > 190.0f && !boost_) {
        setPosition(Vec2(heroX - 190.0f, position_.y));
    }

    if (distance < 80.0f) boost_ = false;

    if (boost_) {
        speed_ = lerp(5.0f, 9.0f, PlayerSkill::get());
    } else {
        float wantedSpeed = lerp(1.0f, 1.9f, PlayerSkill::get());
        speed_ = lerp(speed_, wantedSpeed, 0.05f);
    }

    soundCountdown_ -= speed_;
    if (soundCountdown_ <= 0.0f) {
        soundCountdown_ += 10.0f;
        Sound::playSample("data/sounds/damage");
    }

    setPosition(Vec2(position_.x + speed_, position_.y));

    if (room_->getHero()->Collides(*this)) {
        room_->getHero()->kill();
    }
}
