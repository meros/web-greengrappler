#include "entities/spike.h"
#include "entities/hero.h"
#include "resource.h"
#include "room.h"
#include <cmath>

Spike::Spike() {
    setSize(Vec2(10, 10));
    spikeTile_ = Resource::getBitmap("data/images/tileset1.bmp").subImage(70, 0, 10, 10);
}

void Spike::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(offsetX + position_.x - halfSize_.x));
    int y = static_cast<int>(std::round(offsetY + position_.y - halfSize_.y));
    spikeTile_.draw(renderer, x, y);
}

void Spike::update() {
    if (room_->getHero()->Collides(*this)) {
        room_->getHero()->kill();
    }
}
