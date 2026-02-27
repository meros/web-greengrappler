#include "entities/boss_wall.h"
#include "entities/boss_saw.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"
#include <cmath>

BossWall::BossWall(Direction direction)
    : direction_(direction) {
    setSize(Vec2(10, 100));
    wall_ = Resource::getAnimation("data/images/wall.bmp", 1);
}

void BossWall::setRoom(Room* room) {
    Entity::setRoom(room);
    setTilesCollidable(true);
}

void BossWall::setTilesCollidable(bool v) {
    int sx = static_cast<int>(std::floor((position_.x - halfSize_.x) / 10.0f));
    int sy = static_cast<int>(std::floor((position_.y - halfSize_.y) / 10.0f));
    int height = static_cast<int>(size_.y) / 10;
    for (int y = sy; y < sy + height; y++)
        room_->setCollidable(sx, y, v);
}

void BossWall::onBossWallActivate() {
    active_ = true;
    wallFrameCounter_ = 0;
}

void BossWall::onBossWallDeactivate() {
    active_ = false;
}

void BossWall::onRespawn() {
    active_ = false;
}

void BossWall::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(getDrawPositionX() + offsetX - halfSize_.x));
    int y = static_cast<int>(std::round(getDrawPositionY() + offsetY - halfSize_.y));
    wall_->drawFrame(renderer, 0, x, y, direction_ == Direction::LEFT, false);
}

void BossWall::update() {
    wallFrameCounter_++;

    if (room_->getHero()->Collides(*this)) room_->getHero()->kill();
    if (!active_) return;

    if (direction_ == Direction::RIGHT) {
        if (wallFrameCounter_ % 200 == 0) spawnSaw(-10.0f);
        if ((100 + wallFrameCounter_) % 200 == 0) spawnSaw(-30.0f);
        if ((180 + wallFrameCounter_) % 200 == 0) spawnSaw(-50.0f);
    }

    if (direction_ == Direction::LEFT) {
        if (wallFrameCounter_ % 350 == 0) spawnSaw(-10.0f);
        if ((100 + wallFrameCounter_) % 350 == 0) spawnSaw(-30.0f);
        if ((300 + wallFrameCounter_) % 350 == 0) spawnSaw(-50.0f);
    }
}

void BossWall::spawnSaw(float yOffset) {
    BossSaw* saw = new BossSaw(direction_);
    saw->setPosition(Vec2(position_.x, position_.y + halfSize_.y + yOffset));
    room_->addEntity(saw);
}
