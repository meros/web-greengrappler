#include "entity_factory.h"
#include "constants.h"
#include "entity.h"
#include "entities/hero.h"
#include "entities/reactor.h"
#include "entities/coin.h"
#include "entities/spike.h"
#include "entities/spawn_point.h"
#include "entities/ground_walking_monster.h"
#include "entities/simple_walking_monster.h"
#include "entities/lava_sea.h"
#include "entities/breaking_hook_tile.h"
#include "entities/moving_hook_tile.h"
#include "entities/button_entity.h"
#include "entities/door.h"
#include "entities/wall_of_death.h"
#include "entities/wall_of_death_starter.h"
#include "entities/dialogue.h"
#include "entities/boss_floor.h"
#include "entities/boss_wall.h"
#include "entities/boss.h"

Entity* EntityFactory::create(int id) {
    switch (id) {
        case 0:   return new Hero();
        case 1:   return new Reactor();
        case 2:   return new Coin();
        case 3:   return new Spike();
        case 4:   return new SpawnPoint();
        case 5:   return new GroundWalkingMonster(GroundWalkingMonsterType::FLOOR);
        case 6:   return new GroundWalkingMonster(GroundWalkingMonsterType::ROOF);
        case 7:   return new GroundWalkingMonster(GroundWalkingMonsterType::LEFT_WALL);
        case 8:   return new GroundWalkingMonster(GroundWalkingMonsterType::RIGHT_WALL);
        case 9:   return new SimpleWalkingMonster();
        case 10:  return new LavaSea();
        case 32:  return new BreakingHookTile();
        case 33:  return new Coin();
        case 34:  return new MovingHookTile();
        case 64:  return new ButtonEntity(1);
        case 65:  return new Door(1);
        case 96:  return new WallOfDeath();
        case 97:  return new WallOfDeathStarter();
        case 128: return new Dialogue("data/dialogues/1-tutorial1.txt");
        case 129: return new Dialogue("data/dialogues/2-tutorial2.txt");
        case 160: return new BossFloor();
        case 161: return new BossWall(Direction::RIGHT);
        case 162: return new BossWall(Direction::LEFT);
        case 163: return new Boss();
        default:  return nullptr;
    }
}
