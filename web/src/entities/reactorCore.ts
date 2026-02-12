import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';

export class ReactorCore extends Entity {
  private readonly animation: Animation;
  private frame = 0;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(30, 30));
    this.animation = Resource.getAnimation('data/images/core.bmp', 3);
  }

  getLayer(): number { return 1; }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const pos = new ImmutableVec2(this.getPosition())
      .subtract(new ImmutableVec2(this.animation.getFrameWidth(), this.animation.getFrameHeight()).divide(2))
      .add(new ImmutableVec2(offsetX, offsetY));
    this.animation.drawFrame(ctx, Math.floor(this.frame / 5), Math.round(pos.x), Math.round(pos.y));
  }

  override update(): void {
    this.myVelocity.set(0, this.myVelocity.y + 6.0);
    const bumps = this.moveWithCollision();
    if (bumps.has(Direction.UP) || bumps.has(Direction.DOWN)) {
      this.myVelocity.set(this.myVelocity.x, this.myVelocity.y * 0.8);
    }

    if (this.myRoom.getHero().Collides(this)) {
      if (this.myRoom.getHero().gotCore()) {
        PlayerSkill.playerDidSomethingClever(1.0, 0.75);
        this.myRoom.setCompleted();
        this.remove();
      }
    }
    this.frame++;
  }
}
