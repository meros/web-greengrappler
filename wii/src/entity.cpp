#include "entity.h"
#include "room.h"
#include <cmath>
#include <algorithm>

void Entity::update() {
    frameCounter_++;
}

std::set<Direction> Entity::moveWithCollision(float dx, float dy) {
    float deltaX = (dx <= -99998.0f) ? velocity_.x / TICKS_PER_SECOND : dx;
    float deltaY = (dy <= -99998.0f) ? velocity_.y / TICKS_PER_SECOND : dy;

    int substeps = std::max(1, static_cast<int>(std::ceil((std::abs(deltaX) + std::abs(deltaY)) * 0.2f)));
    deltaX /= substeps;
    deltaY /= substeps;

    std::set<Direction> result;
    float hsx = halfSize_.x;
    float hsy = halfSize_.y;
    int tw = room_->getTileWidth();
    int th = room_->getTileHeight();

    for (int i = 0; i < substeps; i++) {
        // X movement
        position_.x += deltaX;
        int x1 = static_cast<int>(std::floor((position_.x - hsx) / tw));
        int x2 = static_cast<int>(std::floor((position_.x + hsx) / tw));
        int y1n = static_cast<int>(std::floor((position_.y - hsy + 0.01f) / th));
        int y2n = static_cast<int>(std::floor((position_.y + hsy - 0.01f) / th));

        if (deltaX > 0) {
            for (int y = y1n; y <= y2n; y++) {
                if (room_->isCollidable(x2, y)) {
                    deltaX = 0;
                    result.insert(Direction::RIGHT);
                    position_.x = static_cast<float>(x2 * tw) - hsx;
                    break;
                }
            }
        } else if (deltaX < 0) {
            for (int y = y1n; y <= y2n; y++) {
                if (room_->isCollidable(x1, y)) {
                    deltaX = 0;
                    result.insert(Direction::LEFT);
                    position_.x = static_cast<float>((x1 + 1) * tw) + hsx;
                    break;
                }
            }
        }

        // Y movement
        position_.y += deltaY;
        int y1 = static_cast<int>(std::floor((position_.y - hsy) / th));
        int y2 = static_cast<int>(std::floor((position_.y + hsy) / th));
        int x1n = static_cast<int>(std::floor((position_.x - hsx + 0.01f) / tw));
        int x2n = static_cast<int>(std::floor((position_.x + hsx - 0.01f) / tw));

        if (deltaY > 0) {
            for (int x = x1n; x <= x2n; x++) {
                if (room_->isCollidable(x, y2)) {
                    deltaY = 0;
                    result.insert(Direction::DOWN);
                    position_.y = static_cast<float>(y2 * th) - hsy;
                    break;
                }
            }
        } else if (deltaY < 0) {
            for (int x = x1n; x <= x2n; x++) {
                if (room_->isCollidable(x, y1)) {
                    deltaY = 0;
                    result.insert(Direction::UP);
                    position_.y = static_cast<float>((y1 + 1) * th) + hsy;
                    break;
                }
            }
        }
    }
    return result;
}
