import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import { ParticleSystem } from './particleSystem.js';

const enum Facing { LEFT, RIGHT }
const enum MonsterState { IDLING, WALKING }

const IDLE_TO_WALK_CHANCE = 0.05;
const WALK_TO_IDLE_CHANCE = 0.01;

export class SimpleWalkingMonster extends Entity {
  private readonly animation: Animation;
  private facing: Facing = Facing.LEFT;
  private frame = 0;
  private state: MonsterState = MonsterState.IDLING;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(20, 20));
    this.animation = Resource.getAnimation('data/images/robot.bmp', 4);
  }

  getLayer(): number { return 1; }
  override isDamagable(): boolean { return true; }

  private die(): void {
    Sound.playSample('data/sounds/damage');
    const ps = new ParticleSystem(
      Resource.getAnimation('data/images/debris.bmp', 4),
      10, 30, 10, 1, 50, 5, new ImmutableVec2(0, -30), 2.0);
    ps.setPositionWithSpread(this.getPosition(), 5, false);
    this.myRoom.addEntity(ps);
    this.remove();
  }

  override onDamage(): void {
    PlayerSkill.playerDidSomethingClever(0.5, 0.1);
    this.die();
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getPosition().x - this.animation.getFrameWidth() / 2 + offsetX);
    const y = Math.round(this.getPosition().y - this.animation.getFrameHeight() / 2 + offsetY);
    this.animation.drawFrame(ctx, Math.floor(this.frame / 15), x, y,
      this.facing === Facing.RIGHT, false);
  }

  override update(): void {
    switch (this.state) {
      case MonsterState.WALKING: {
        this.myVelocity.set(20 * (this.facing === Facing.LEFT ? -1 : 1), this.myVelocity.y + 6.0);
        this.setVelocity(this.myVelocity);
        const bumps = this.moveWithCollision();

        if (bumps.has(Direction.UP) || bumps.has(Direction.DOWN))
          this.myVelocity.set(this.myVelocity.x, 0);
        if (bumps.has(Direction.LEFT)) this.facing = Facing.RIGHT;
        if (bumps.has(Direction.RIGHT)) this.facing = Facing.LEFT;

        const ox = this.facing === Facing.RIGHT ? -this.getHalfSize().x - 2 : this.getHalfSize().x + 2;
        const oy = this.getHalfSize().y + 2;
        const tx = Math.floor((this.getPosition().x + ox) / this.myRoom.getTileWidth());
        const ty = Math.floor((this.getPosition().y + oy) / this.myRoom.getTileHeight());

        if (!this.myRoom.isCollidable(tx, ty)) {
          this.facing = this.facing === Facing.LEFT ? Facing.RIGHT : Facing.LEFT;
        }

        if (Math.random() < WALK_TO_IDLE_CHANCE) this.state = MonsterState.IDLING;
        break;
      }
      case MonsterState.IDLING:
        if (Math.random() < IDLE_TO_WALK_CHANCE) {
          this.facing = Math.random() > 0.5 ? Facing.LEFT : Facing.RIGHT;
          this.state = MonsterState.WALKING;
        }
        break;
    }

    const hero = this.myRoom.getHero();
    if (hero.Collides(this)) hero.kill();
    this.frame++;
  }
}
