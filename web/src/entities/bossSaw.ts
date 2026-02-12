import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';

export class BossSaw extends Entity {
  private readonly lifeTime = 60 * 6;
  private frameCounter = 0;
  private readonly saw: Animation;
  private readonly speed: ImmutableVec2;

  constructor(direction: Direction) {
    super();
    this.speed = direction === Direction.LEFT
      ? new ImmutableVec2(-2, 0)
      : new ImmutableVec2(2, 0);
    Sound.playSample('data/sounds/boss_saw');
    this.saw = Resource.getAnimation('data/images/saw.bmp');
    this.setSize(new ImmutableVec2(this.saw.getFrameWidth(), this.saw.getFrameHeight()));
  }

  getLayer(): number { return 3; }

  override onRespawn(): void { this.remove(); }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getDrawPositionX() + offsetX - this.getHalfSize().x);
    const y = Math.round(this.getDrawPositionY() + offsetY - this.getHalfSize().y);
    this.saw.drawFrame(ctx, Math.floor(this.frameCounter / 10), x, y);
  }

  override update(): void {
    this.frameCounter++;
    if (this.frameCounter > this.lifeTime) this.remove();
    this.setPosition(new ImmutableVec2(this.getPosition()).add(this.speed));
    if (this.myRoom.getHero().Collides(this)) this.myRoom.getHero().kill();
  }
}
