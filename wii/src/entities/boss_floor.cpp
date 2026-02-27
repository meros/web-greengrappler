#include "entities/boss_floor.h"
#include "entities/hero.h"
#include "room.h"

BossFloor::BossFloor() {
    setSize(Vec2(320, 10));
}

void BossFloor::onBossFloorActivate() {
    active_ = true;
    floorFrameCounter_ = 0;
}

void BossFloor::onRespawn() {
    active_ = false;
}

void BossFloor::draw(SDL_Renderer* /*renderer*/, int /*offsetX*/, int /*offsetY*/, int /*layer*/) {
    // Invisible entity
}

void BossFloor::update() {
    floorFrameCounter_++;
    if (!active_) return;
    if (room_->getHero()->Collides(*this)) room_->getHero()->kill();
    if (floorFrameCounter_ > 60) active_ = false;
}
