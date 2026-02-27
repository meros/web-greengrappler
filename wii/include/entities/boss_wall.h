#pragma once
#include "entity.h"
#include "constants.h"

class Animation;
class Room;

class BossWall : public Entity {
public:
    BossWall(Direction direction);

    int getLayer() const override { return 3; }

    void setRoom(Room* room) override;
    void onBossWallActivate() override;
    void onBossWallDeactivate() override;
    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    void setTilesCollidable(bool v);
    void spawnSaw(float yOffset);

    Animation* wall_;
    Direction direction_;
    int wallFrameCounter_ = 0;
    bool active_ = false;
};
