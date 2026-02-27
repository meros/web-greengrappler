#pragma once
#include "entity.h"
#include "vec2.h"
#include <vector>

class Animation;

class ParticleSystem : public Entity {
public:
    ParticleSystem(Animation* animation, int animSpeed, int lifeTime, int blinkTime,
                   float minSpeed, float maxSpeed, int numParticles,
                   Vec2 initialVel, float gravity);

    int getLayer() const override { return 3; }

    void onRespawn() override;
    void setPosition(Vec2 pos);
    void setPositionWithSpread(Vec2 position, float randomAmount, bool randomVel);
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    Animation* animation_;
    int animationSpeed_;
    int blinkTimeTicks_;
    Vec2 gravity_;
    Vec2 initialVel_;
    float maxSpeed_;
    float minSpeed_;
    int numParticles_;
    std::vector<Vec2> particlesPos_;
    std::vector<Vec2> particlesVel_;
    int lifeTimeTicks_;
    int frameNum_ = 0;
};
