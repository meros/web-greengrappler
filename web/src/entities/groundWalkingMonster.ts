import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { lerp } from '../utils.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import { collides } from '../collision.js';

const enum Facing { LEFT_UP, RIGHT_DOWN }

export const enum GroundWalkingMonsterType { FLOOR, LEFT_WALL, RIGHT_WALL, ROOF }

const MIN_WALKING_SPEED = 40.0;
const MAX_WALKING_SPEED = 70.0;

export class GroundWalkingMonster extends Entity {
  private readonly animation: Animation;
  private facing: Facing = Facing.RIGHT_DOWN;
  private frame = 0;
  private readonly type: GroundWalkingMonsterType;

  constructor(type: GroundWalkingMonsterType) {
    super();
    this.type = type;
    this.setSize(new ImmutableVec2(10, 10));
    this.animation = Resource.getAnimation('data/images/saw.bmp', 2);
  }

  getLayer(): number { return 1; }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getPosition().x - this.animation.getFrameWidth() / 2 + offsetX);
    const y = Math.round(this.getPosition().y - this.animation.getFrameHeight() / 2 + offsetY);
    this.animation.drawFrame(ctx, Math.floor(this.frame / 3), x, y);
  }

  override update(): void {
    const hero = this.myRoom.getHero();
    const speed = lerp(MIN_WALKING_SPEED, MAX_WALKING_SPEED, PlayerSkill.get());

    if (this.type === GroundWalkingMonsterType.FLOOR || this.type === GroundWalkingMonsterType.ROOF) {
      if (this.facing === Facing.LEFT_UP) this.getVelocity().set(-speed, 0);
      else this.getVelocity().set(speed, 0);
    } else {
      if (this.facing === Facing.LEFT_UP) this.getVelocity().set(0, -speed);
      else this.getVelocity().set(0, speed);
    }

    const bumps = this.moveWithCollision();

    if (bumps.has(Direction.LEFT) || bumps.has(Direction.UP)) this.facing = Facing.RIGHT_DOWN;
    if (bumps.has(Direction.RIGHT) || bumps.has(Direction.DOWN)) this.facing = Facing.LEFT_UP;

    let offsetX: number;
    let offsetY: number;

    if (this.type === GroundWalkingMonsterType.FLOOR || this.type === GroundWalkingMonsterType.ROOF) {
      offsetX = this.facing === Facing.LEFT_UP ? -this.getHalfSize().x - 2 : this.getHalfSize().x + 2;
      offsetY = this.type === GroundWalkingMonsterType.FLOOR ? this.getHalfSize().y + 2 : -this.getHalfSize().y - 2;
    } else {
      offsetX = this.type === GroundWalkingMonsterType.LEFT_WALL ? -this.getHalfSize().x - 2 : this.getHalfSize().x + 2;
      offsetY = this.facing === Facing.RIGHT_DOWN ? this.getHalfSize().y + 2 : -this.getHalfSize().y - 2;
    }

    const tx = Math.floor((this.getPosition().x + offsetX) / this.myRoom.getTileWidth());
    const ty = Math.floor((this.getPosition().y + offsetY) / this.myRoom.getTileHeight());

    if (!this.myRoom.isCollidable(tx, ty)) {
      this.facing = this.facing === Facing.LEFT_UP ? Facing.RIGHT_DOWN : Facing.LEFT_UP;
    }

    if (hero.Collides(this)) hero.kill();

    if (hero.hasHook() && collides(this, hero.getHookCollidable())) {
      Sound.playSample('data/sounds/hook');
      hero.detachHook();
    }

    this.frame++;
  }
}
