#include "entities/door.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"
#include <cmath>

Door::Door(int id)
    : id_(id) {
    setSize(Vec2(10, 40));
    doorAnim_ = Resource::getAnimation("data/images/door.bmp", 1);
}

void Door::onRespawn() {
    opening_ = false;
    closing_ = false;
    doorHeight_ = 40;
}

void Door::onButtonDown(int id) {
    if (id_ != id) return;
    doorFrameCounter_ = 0;
    opening_ = true;
    closing_ = false;
}

void Door::onButtonUp(int id) {
    if (id_ != id) return;
    closing_ = true;
    opening_ = false;
    doorFrameCounter_ = 0;
}

void Door::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(getDrawPositionX() + offsetX - size_.x / 2.0f));
    int y = static_cast<int>(std::round(getDrawPositionY() + offsetY - size_.y / 2.0f));

    GameImage img = doorAnim_->getGameImage();
    GameImage sub = img.subImage(0, 0, 10, doorHeight_);
    sub.draw(renderer, x, y);
}

void Door::update() {
    doorFrameCounter_++;

    if (doorHeight_ < 4) doorHeight_ = 4;

    if (opening_ && doorHeight_ != 4) {
        doorHeight_ -= 2;
    } else if (opening_) {
        doorFrameCounter_ = 0;
    }

    if (closing_ && doorFrameCounter_ % 5 == 0 && doorHeight_ != 40) {
        doorHeight_++;
    }

    int tileX = static_cast<int>(std::floor((position_.x - size_.x / 2.0f) / 10.0f));
    int tileY = static_cast<int>(std::floor((position_.y - size_.y / 2.0f) / 10.0f));

    for (int i = 0; i < 4; i++) {
        room_->setCollidable(tileX, tileY + i, i <= doorHeight_ / 10);
    }
}
