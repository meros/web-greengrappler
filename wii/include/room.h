#pragma once
#include "vec2.h"
#include <vector>
#include <SDL2/SDL.h>

class Hero;
class Coin;
class Entity;
class Camera;
class Layer;
class BitmapFont;
class Animation;
struct ParticleSystem;

class Room {
public:
    Room(Layer* bg, Layer* mid, Layer* fg, Camera* camera);
    ~Room();

    void addEntity(Entity* entity);

    Hero* getHero() const { return hero_; }
    Camera* getCamera() const { return camera_; }
    const std::vector<Entity*>& getEntities() const { return entities_; }

    int getTileWidth() const { return 10; }
    int getTileHeight() const { return 10; }
    int getWidthInTiles() const;
    int getHeightInTiles() const;

    bool isCollidable(int x, int y) const;
    bool isHookable(int x, int y) const;
    void setCollidable(int x, int y, bool v);
    void setHookable(int x, int y, bool v);

    bool isCompleted() const;
    void setCompleted();

    bool damageDone(int x, int y);
    Entity* findHookableEntity(Vec2 position);
    bool rayCast(Vec2 origin, Vec2 direction, bool cull, int& outX, int& outY);

    void broadcastButtonDown(int id);
    void broadcastButtonUp(int id);
    void broadcastStartWallOfDeath();
    void broadcastBossWallActivate();
    void broadcastBossWallDeactivate();
    void broadcastBossFloorActivate();

    void destroyToTileRow(int x);

    void onDraw(SDL_Renderer* renderer);
    void onLogic();

private:
    std::vector<Entity*> entities_;
    std::vector<Entity*> damagableEntities_;
    std::vector<Entity*> hookableEntities_;
    std::vector<Coin*> staticCoins_;
    std::vector<Entity*> entitiesToAdd_;

    Hero* hero_ = nullptr;

    Layer* backgroundLayer_;
    Layer* middleLayer_;
    Layer* foregroundLayer_;
    Camera* camera_;

    std::vector<Layer*> bgLayers_;
    std::vector<Layer*> midLayers_;
    std::vector<Layer*> fgLayers_;

    std::vector<std::vector<bool>> hookableArray_;
    std::vector<std::vector<bool>> collidableArray_;

    int destroyedToTileRow_ = -1;
    int frameCounter_ = 0;
    bool heroIsDead_ = false;
    bool isCompletedFlag_ = false;

    BitmapFont* font_;
};
