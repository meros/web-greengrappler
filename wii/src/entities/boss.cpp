#include "entities/boss.h"
#include "entities/particle_system.h"
#include "entities/reactor_core.h"
#include "entities/coin.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "media/music.h"
#include "room.h"
#include "camera.h"
#include "player_skill.h"
#include "utils.h"
#include <cmath>
#include <cstdlib>

static const int BLOW_TIME = 60 * 4 + 30;
static const int DAMAGE_MAX = 16;
static const int FRAME_PER_DAMAGE = 4;
static const int INITIAL_BLOW_TIME = 60 * 4 + 30;
static const float MAX_HEALTH = 40.0f;
static const float MIN_HEALTH = 10.0f;

Boss::Boss()
    : health_(lerp(MAX_HEALTH, MIN_HEALTH, PlayerSkill::get())) {
    setSize(Vec2(30, 40));
    reactorAnim_ = Resource::getAnimation("data/images/reactor_shell.bmp", 4);
    bossAnim_ = Resource::getAnimation("data/images/boss.bmp", 4);
}

void Boss::setRoom(Room* room) {
    Entity::setRoom(room);
    setTilesCollidable(true);
}

void Boss::setTilesCollidable(bool v) {
    int sx = static_cast<int>(std::floor((position_.x - halfSize_.x) / 10.0f));
    int sy = static_cast<int>(std::floor((position_.y - halfSize_.y) / 10.0f));
    for (int x = sx; x < sx + 3; x++)
        for (int y = sy; y < sy + 4; y++)
            room_->setCollidable(x, y, v);
}

void Boss::spawnDebris() {
    int numPs = std::abs(rand() % 5) + 1;
    ParticleSystem* ps = new ParticleSystem(
        Resource::getAnimation("data/images/debris.bmp", 4),
        10, 40, 10, 1.0f, 50.0f, numPs, Vec2(0, -20), 2.0f);
    ps->setPositionWithSpread(position_, 10.0f, false);
    room_->addEntity(ps);
}

void Boss::onDamage() {
    if (state_ == BossState::SLEEPING) {
        spawnDebris();
        initialHealth_--;
        if (initialHealth_ == 0) {
            bossFrameCounter_ = 0;
            state_ = BossState::INITIAL_BLOW;
            Sound::playSample("data/sounds/damage");
            room_->getCamera()->addShake(4.0f, static_cast<float>(INITIAL_BLOW_TIME));
        } else {
            Sound::playSample("data/sounds/damage");
            room_->getCamera()->addShake(1.0f, 20.0f);
        }
    }

    if (state_ == BossState::VULNERABLE) {
        spawnDebris();
        health_ -= 1.0f;
        if (health_ < 0) {
            state_ = BossState::DEAD;
            bossFrameCounter_ = 0;
            Sound::playSample("data/sounds/damage");
            room_->getCamera()->addShake(4.0f, static_cast<float>(BLOW_TIME));
        } else {
            Sound::playSample("data/sounds/damage");
            room_->getCamera()->addShake(1.0f, 20.0f);
        }
    }
}

void Boss::onRespawn() {
    setPosition(originalPos_);
    state_ = BossState::SLEEPING;
    direction_ = Direction::LEFT;
    health_ = lerp(MAX_HEALTH, MIN_HEALTH, PlayerSkill::get());
    initialHealth_ = 16;
    animFrameCounter_ = 0;
    Music::popSong();
    setTilesCollidable(true);
}

void Boss::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(getDrawPositionX() + offsetX - halfSize_.x));
    int y = static_cast<int>(std::round(getDrawPositionY() + offsetY - halfSize_.y));

    if (state_ == BossState::SLEEPING) {
        int frame = DAMAGE_MAX / FRAME_PER_DAMAGE - (initialHealth_ + 3) / FRAME_PER_DAMAGE;
        reactorAnim_->drawFrame(renderer, frame, x, y);
    } else if (state_ == BossState::INITIAL_BLOW) {
        reactorAnim_->drawFrame(renderer, 3, x, y);
    } else if (state_ == BossState::VULNERABLE) {
        bossAnim_->drawFrame(renderer, 0, x, y);
    } else {
        bossAnim_->drawFrame(renderer, animFrameCounter_ / 5, x, y);
    }
}

void Boss::update() {
    animFrameCounter_++;
    bossFrameCounter_++;

    if (state_ == BossState::INIT) {
        originalPos_ = position_;
        state_ = BossState::SLEEPING;
    }

    if (state_ != BossState::MOVE_UPWARDS && state_ != BossState::SLEEPING &&
        state_ != BossState::INITIAL_BLOW && state_ != BossState::DEAD &&
        state_ != BossState::VULNERABLE) {
        if (room_->getHero()->Collides(*this)) room_->getHero()->kill();
    }

    if (state_ == BossState::INITIAL_BLOW) {
        if (bossFrameCounter_ % 60 == 0) Sound::playSample("data/sounds/alarm");
        if (bossFrameCounter_ >= INITIAL_BLOW_TIME) {
            Sound::playSample("data/sounds/reactor_explosion");
            Sound::playSample("data/sounds/start");
            ParticleSystem* ps = new ParticleSystem(
                Resource::getAnimation("data/images/debris.bmp", 4),
                20, 200, 20, 1.0f, 50.0f, 50, Vec2(0, -150), 5.0f);
            ps->setPositionWithSpread(position_, 10.0f, false);
            room_->addEntity(ps);
            bossFrameCounter_ = 0;
            state_ = BossState::AWAKENING;
        }
    }

    if (state_ == BossState::AWAKENING) {
        if (bossFrameCounter_ > 60 * 3) {
            state_ = BossState::MOVE_UPWARDS;
            bossFrameCounter_ = 0;
            setTilesCollidable(false);
            Music::pushSong();
            Music::playSong("data/music/olof3.xm");
        }
    }

    if (state_ == BossState::MOVE_UPWARDS) {
        setPosition(Vec2(position_.x, position_.y - 1.0f));
        if (originalPos_.y - position_.y > 130.0f) {
            state_ = BossState::FLOAT;
            room_->broadcastBossWallActivate();
            bossFrameCounter_ = 0;
        }
    }

    if (state_ == BossState::FLOAT) {
        float yDelta = std::sin(bossFrameCounter_ / 8.0f) * 2.0f;
        if (direction_ == Direction::RIGHT) {
            setPosition(Vec2(position_.x + 1.5f, position_.y + yDelta));
            if (position_.x - originalPos_.x > 110.0f) direction_ = Direction::LEFT;
        }
        if (direction_ == Direction::LEFT) {
            setPosition(Vec2(position_.x - 1.5f, position_.y + yDelta));
            if (originalPos_.x - position_.x > 110.0f) direction_ = Direction::RIGHT;
        }
        if (bossFrameCounter_ > 60 * 7) {
            state_ = BossState::ATTACK;
            bossFrameCounter_ = 0;
        }
    }

    if (state_ == BossState::ATTACK) {
        if (bossFrameCounter_ < 40) {
            if (bossFrameCounter_ % 2 == 0)
                setPosition(Vec2(position_.x - 2.0f, position_.y));
            else
                setPosition(Vec2(position_.x + 2.0f, position_.y));
        } else {
            setPosition(Vec2(position_.x, position_.y + 6.0f));
            if (originalPos_.y < position_.y) {
                room_->broadcastBossWallDeactivate();
                setPosition(Vec2(position_.x, originalPos_.y));
                state_ = BossState::VULNERABLE;
                room_->getCamera()->addShake(5.0f, 80.0f);

                int numPs = std::abs(rand() % 10) + 5;
                ParticleSystem* ps = new ParticleSystem(
                    Resource::getAnimation("data/images/debris.bmp", 4),
                    10, 40, 10, 1.0f, 50.0f, numPs, Vec2(0, -20), 2.0f);
                ps->setPositionWithSpread(position_, 10.0f, false);
                room_->addEntity(ps);
                room_->broadcastBossFloorActivate();
                Sound::playSample("data/sounds/reactor_explosion");
                int coinCount = std::abs(rand() % 5) + 3;
                Coin::spawnDeathCoins(coinCount, position_, 60 * 6, room_);
            }
        }
    }

    if (state_ == BossState::VULNERABLE) {
        if (bossFrameCounter_ > 60 * 5) {
            state_ = BossState::MOVE_UPWARDS;
            bossFrameCounter_ = 0;
        }
    }

    if (state_ == BossState::DEAD) {
        if (bossFrameCounter_ % 60 == 0) Sound::playSample("data/sounds/alarm");
        if (bossFrameCounter_ >= BLOW_TIME) {
            Sound::playSample("data/sounds/reactor_explosion");
            Sound::playSample("data/sounds/start");
            ParticleSystem* ps = new ParticleSystem(
                Resource::getAnimation("data/images/debris.bmp", 4),
                20, 200, 20, 1.0f, 50.0f, 50, Vec2(0, -150), 5.0f);
            ps->setPositionWithSpread(position_, 10.0f, false);
            room_->addEntity(ps);
            ReactorCore* core = new ReactorCore();
            core->setPosition(position_);
            core->setVelocity(Vec2(0, -50));
            room_->addEntity(core);
            remove();
        }
    }
}
