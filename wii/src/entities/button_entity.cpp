#include "entities/button_entity.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"

ButtonEntity::ButtonEntity(int id)
    : id_(id) {
    counter_ = time_ + 1;
    setSize(Vec2(10, 10));
    buttonAnim_ = Resource::getAnimation("data/images/button.bmp");
}

void ButtonEntity::onRespawn() {
    triggered_ = false;
    counter_ = time_ + 1;
}

void ButtonEntity::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = getDrawPositionX() + offsetX;
    int y = getDrawPositionY() + offsetY;
    int frame = (counter_ > time_) ? 0 : 1;
    buttonAnim_->drawFrame(renderer, frame,
        x - static_cast<int>(size_.x) / 2,
        y - static_cast<int>(size_.y) / 2);
}

void ButtonEntity::update() {
    counter_++;

    if (counter_ == time_) {
        Sound::playSample("data/sounds/timeout");
        room_->broadcastButtonUp(id_);
        return;
    }

    if (counter_ < time_) {
        if (counter_ % 60 == 0) Sound::playSample("data/sounds/time");
        return;
    }

    if (room_->getHero()->Collides(*this)) {
        if (!triggered_) {
            triggered_ = true;
            counter_ = 0;
            Sound::playSample("data/sounds/time");
            room_->broadcastButtonDown(id_);
        }
    } else {
        triggered_ = false;
    }
}
