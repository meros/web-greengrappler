#include "entities/reactor.h"
#include "entities/particle_system.h"
#include "entities/reactor_core.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"
#include "camera.h"
#include <cmath>
#include <cstdlib>

static const int BLOW_TIME = 60 * 4 + 30;
static const int DAMAGE_MAX = 16;
static const int FRAME_PER_DAMAGE = 4;

Reactor::Reactor() {
    setSize(Vec2(30, 40));
    shell_ = Resource::getAnimation("data/images/reactor_shell.bmp", 4);
}

void Reactor::setRoom(Room* room) {
    Entity::setRoom(room);
    setTilesCollidable(true);
}

void Reactor::setTilesCollidable(bool v) {
    int sx = static_cast<int>(std::floor((position_.x - halfSize_.x) / 10.0f));
    int sy = static_cast<int>(std::floor((position_.y - halfSize_.y) / 10.0f));
    for (int x = sx; x < sx + 3; x++)
        for (int y = sy; y < sy + 4; y++)
            room_->setCollidable(x, y, v);
}

void Reactor::onDamage() {
    if (damage_ >= DAMAGE_MAX) return;

    int numPs = std::abs(rand() % 5) + 1;
    ParticleSystem* ps = new ParticleSystem(
        Resource::getAnimation("data/images/debris.bmp", 4),
        10, 40, 10, 1.0f, 50.0f, numPs, Vec2(0, -20), 2.0f);
    ps->setPositionWithSpread(position_, 10.0f, false);
    room_->addEntity(ps);

    damage_++;
    if (damage_ == DAMAGE_MAX) {
        reactorFrameCounter_ = 1;
        aboutToBlow_ = true;
        Sound::playSample("data/sounds/damage");
        room_->getCamera()->addShake(4.0f, static_cast<float>(BLOW_TIME));
    } else {
        Sound::playSample("data/sounds/damage");
        room_->getCamera()->addShake(1.0f, 20.0f);
    }
}

void Reactor::onRespawn() {
    aboutToBlow_ = false;
    damage_ = 0;
}

void Reactor::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = getDrawPositionX() + offsetX;
    int y = getDrawPositionY() + offsetY;
    int frame = (damage_ >= DAMAGE_MAX)
        ? damage_ / FRAME_PER_DAMAGE - 1
        : damage_ / FRAME_PER_DAMAGE;
    shell_->drawFrame(renderer, frame,
        x - static_cast<int>(size_.x) / 2,
        y - static_cast<int>(size_.y) / 2);
}

void Reactor::update() {
    reactorFrameCounter_++;

    if (aboutToBlow_ && reactorFrameCounter_ % 60 == 0)
        Sound::playSample("data/sounds/alarm");

    if (aboutToBlow_ && reactorFrameCounter_ % 20 == 0) {
        int numPs = std::abs(rand() % 5) + 1;
        ParticleSystem* ps = new ParticleSystem(
            Resource::getAnimation("data/images/debris.bmp", 4),
            10, 40, 10, 1.0f, 50.0f, numPs, Vec2(0, -20), 2.0f);
        ps->setPositionWithSpread(position_, 10.0f, false);
        room_->addEntity(ps);
    }

    if (aboutToBlow_ && reactorFrameCounter_ >= BLOW_TIME) {
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

        setTilesCollidable(false);
        remove();
    }
}
