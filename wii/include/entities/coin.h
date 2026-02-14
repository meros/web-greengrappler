#pragma once
#include "entity.h"
#include "vec2.h"

class Animation;
class Room;

enum class CoinType { DYNAMIC, STATIC };

class Coin : public Entity {
public:
    Coin();

    int getLayer() const override { return 3; }

    static void spawnDeathCoins(int count, Vec2 center, int lifeTime, Room* room);

    void setLifeTime(int lt);
    void setCollides();
    bool isDynamic() const { return type_ == CoinType::DYNAMIC; }

    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    Animation* animationCoin_;
    int frame_ = 0;
    int lifeTime_ = 0;
    bool temporary_ = false;
    CoinType type_ = CoinType::STATIC;
    bool collides_ = false;
};
