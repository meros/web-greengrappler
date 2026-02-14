#include "entities/coin.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"
#include "player_skill.h"
#include "utils.h"
#include <cmath>

Coin::Coin() {
    animationCoin_ = Resource::getAnimation("data/images/coin.bmp", 4);
    setSize(Vec2(12, 12));
}

void Coin::spawnDeathCoins(int count, Vec2 center, int lifeTime, Room* room) {
    for (int i = 0; i < count; i++) {
        Vec2 vel = sincos(static_cast<float>(M_PI) * (i + 1) / (count + 1));
        Vec2 velocity(vel.x * 100.0f, vel.y * -200.0f);
        Coin* coin = new Coin();
        coin->setLifeTime(lifeTime);
        coin->setPosition(center);
        coin->setVelocity(velocity);
        room->addEntity(coin);
    }
}

void Coin::setLifeTime(int lt) {
    lifeTime_ = lt;
    type_ = CoinType::DYNAMIC;
    temporary_ = (lt != 0);
}

void Coin::setCollides() {
    collides_ = true;
}

void Coin::onRespawn() {
    if (type_ == CoinType::DYNAMIC) remove();
}

void Coin::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(position_.x)) - animationCoin_->getFrameWidth() / 2 + offsetX;
    int y = static_cast<int>(std::round(position_.y)) - animationCoin_->getFrameHeight() / 2 + offsetY;

    if (!temporary_ || (lifeTime_ / 10) % 2 == 0) {
        int f = frame_;
        if (f > 180) f = 0;
        int frameIdx = (f < 5 * 4) ? (f / 5) : 0;
        animationCoin_->drawFrame(renderer, frameIdx, x, y);
    }
}

void Coin::update() {
    if (type_ == CoinType::DYNAMIC) {
        if (room_->getHero()->Collides(*this)) {
            setCollides();
        }

        if (temporary_ && lifeTime_ == 0) {
            remove();
        } else {
            velocity_.y += 6.0f;
            auto bumps = moveWithCollision();

            if (bumps.count(Direction::LEFT) || bumps.count(Direction::RIGHT)) {
                if (std::abs(velocity_.x) > 10.0f) Sound::playSample("data/sounds/coin");
                velocity_.x *= -0.5f;
            }
            if (bumps.count(Direction::UP) || bumps.count(Direction::DOWN)) {
                if (std::abs(velocity_.y) > 10.0f) Sound::playSample("data/sounds/coin");
                velocity_.y *= -0.2f;
            }
            lifeTime_--;
        }
    }

    if (collides_) {
        if (room_->getHero()->gotCoin()) {
            PlayerSkill::playerDidSomethingClever(0.3f, 0.05f);
            Sound::playSample("data/sounds/coin");
            remove();
        }
    }

    frame_++;
}
