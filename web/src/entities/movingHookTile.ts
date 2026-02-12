import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';
import type { Room } from '../room.js';
import type { ReadonlyVec2 } from '../math.js';

export class MovingHookTile extends Entity {
  private readonly sprite: Animation;
  private hasHook = false;
  private initialPosition = new ImmutableVec2();
  private initialVelocity = new ImmutableVec2();

  constructor() {
    super();
    this.sprite = Resource.getAnimation('data/images/movinghooktile.bmp', 2);
    this.setSize(new ImmutableVec2(this.sprite.getFrameHeight(), this.sprite.getFrameHeight()));
    this.myVelocity.set(20, 0);
  }

  getLayer(): number { return 0; }
  override isHookable(): boolean { return true; }

  override setRoom(room: Room): void {
    this.initialPosition = new ImmutableVec2(this.getPosition());
    this.initialVelocity = new ImmutableVec2(this.myVelocity);
    super.setRoom(room);
  }

  override onRespawn(): void {
    this.setPosition(this.initialPosition);
    this.myVelocity.set(this.initialVelocity);
    this.hasHook = false;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = this.getDrawPositionX() + offsetX - this.sprite.getFrameWidth() / 2;
    const y = this.getDrawPositionY() + offsetY - this.sprite.getFrameHeight() / 2;
    this.sprite.drawFrame(ctx, Math.floor(this.myFrameCounter / 12), x, y);
  }

  override update(): void {
    super.update();

    this.hasHook = (this.myRoom.getHero().getHookedEntity() === this);

    if (this.hasHook) {
      this.myVelocity.set(this.myVelocity.x > 0 ? 40 : -40, this.myVelocity.y);
      this.myFrameCounter++;
    } else {
      this.myVelocity.set(this.myVelocity.x > 0 ? 20 : -20, this.myVelocity.y);
    }

    const bumps = this.moveWithCollision();
    if (bumps.has(Direction.LEFT) || bumps.has(Direction.RIGHT)) {
      this.myVelocity.set(-this.myVelocity.x, this.myVelocity.y);
    }
  }
}
