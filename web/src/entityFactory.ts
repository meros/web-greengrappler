import { Direction } from './constants.js';
import type { Entity } from './entity.js';
import { Hero } from './entities/hero.js';
import { Reactor } from './entities/reactor.js';
import { Coin } from './entities/coin.js';
import { Spike } from './entities/spike.js';
import { SpawnPoint } from './entities/spawnPoint.js';
import { GroundWalkingMonster, GroundWalkingMonsterType } from './entities/groundWalkingMonster.js';
import { SimpleWalkingMonster } from './entities/simpleWalkingMonster.js';
import { LavaSea } from './entities/lavaSea.js';
import { BreakingHookTile } from './entities/breakingHookTile.js';
import { MovingHookTile } from './entities/movingHookTile.js';
import { Button } from './entities/button.js';
import { Door } from './entities/door.js';
import { WallOfDeath } from './entities/wallOfDeath.js';
import { WallOfDeathStarter } from './entities/wallOfDeathStarter.js';
import { Dialogue } from './entities/dialogue.js';
import { BossFloor } from './entities/bossFloor.js';
import { BossWall } from './entities/bossWall.js';
import { Boss } from './entities/boss.js';

export const EntityFactory = {
  create(id: number): Entity | null {
    switch (id) {
      case 0: return new Hero();
      case 1: return new Reactor();
      case 2: return new Coin();
      case 3: return new Spike();
      case 4: return new SpawnPoint();
      case 5: return new GroundWalkingMonster(GroundWalkingMonsterType.FLOOR);
      case 6: return new GroundWalkingMonster(GroundWalkingMonsterType.ROOF);
      case 7: return new GroundWalkingMonster(GroundWalkingMonsterType.LEFT_WALL);
      case 8: return new GroundWalkingMonster(GroundWalkingMonsterType.RIGHT_WALL);
      case 9: return new SimpleWalkingMonster();
      case 10: return new LavaSea();
      case 32: return new BreakingHookTile();
      case 33: return new Coin();
      case 34: return new MovingHookTile();
      case 64: return new Button(1);
      case 65: return new Door(1);
      case 96: return new WallOfDeath();
      case 97: return new WallOfDeathStarter();
      case 128: return new Dialogue('data/dialogues/1-tutorial1.txt');
      case 129: return new Dialogue('data/dialogues/2-tutorial2.txt');
      case 160: return new BossFloor();
      case 161: return new BossWall(Direction.RIGHT);
      case 162: return new BossWall(Direction.LEFT);
      case 163: return new Boss();
      default: return null;
    }
  },
};
