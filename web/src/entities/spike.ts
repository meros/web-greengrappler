import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import type { GameImage } from '../resource.js';

export class Spike extends Entity {
  private readonly spikeTile: GameImage;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(10, 10));
    this.spikeTile = Resource.getBitmap('data/images/tileset1.bmp').subImage(70, 0, 10, 10);
  }

  getLayer(): number { return 3; }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    this.spikeTile.draw(ctx,
      Math.round(offsetX + this.getPosition().x - this.getHalfSize().x),
      Math.round(offsetY + this.getPosition().y - this.getHalfSize().y));
  }

  override update(): void {
    if (this.myRoom.getHero().Collides(this)) {
      this.myRoom.getHero().kill();
    }
  }
}
