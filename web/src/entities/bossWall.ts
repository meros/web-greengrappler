import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';
import type { Room } from '../room.js';
import { BossSaw } from './bossSaw.js';

export class BossWall extends Entity {
  private readonly wall: Animation;
  private readonly direction: Direction;
  private frameCounter = 0;
  private active = false;

  constructor(direction: Direction) {
    super();
    this.direction = direction;
    this.setSize(new ImmutableVec2(10, 100));
    this.wall = Resource.getAnimation('data/images/wall.bmp', 1);
  }

  getLayer(): number { return 3; }

  override setRoom(room: Room): void {
    super.setRoom(room);
    this.setTilesCollidable(true);
  }

  private setTilesCollidable(v: boolean): void {
    const sx = Math.floor((this.getPosition().x - this.getHalfSize().x) / 10);
    const sy = Math.floor((this.getPosition().y - this.getHalfSize().y) / 10);
    const height = Math.floor(this.getSize().y / 10);
    for (let y = sy; y < sy + height; y++)
      this.myRoom.setCollidable(sx, y, v);
  }

  override onBossWallActivate(): void {
    this.active = true;
    this.frameCounter = 0;
  }

  override onBossWallDeactivate(): void { this.active = false; }
  override onRespawn(): void { this.active = false; }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getDrawPositionX() + offsetX - this.getHalfSize().x);
    const y = Math.round(this.getDrawPositionY() + offsetY - this.getHalfSize().y);
    this.wall.drawFrame(ctx, 0, x, y, this.direction === Direction.LEFT, false);
  }

  override update(): void {
    this.frameCounter++;

    if (this.myRoom.getHero().Collides(this)) this.myRoom.getHero().kill();
    if (!this.active) return;

    if (this.direction === Direction.RIGHT) {
      if (this.frameCounter % 200 === 0) this.spawnSaw(-10);
      if ((100 + this.frameCounter) % 200 === 0) this.spawnSaw(-30);
      if ((180 + this.frameCounter) % 200 === 0) this.spawnSaw(-50);
    }

    if (this.direction === Direction.LEFT) {
      if (this.frameCounter % 350 === 0) this.spawnSaw(-10);
      if ((100 + this.frameCounter) % 350 === 0) this.spawnSaw(-30);
      if ((300 + this.frameCounter) % 350 === 0) this.spawnSaw(-50);
    }
  }

  private spawnSaw(yOffset: number): void {
    const saw = new BossSaw(this.direction);
    saw.setPosition(new ImmutableVec2(this.getPosition().x, this.getPosition().y + this.getHalfSize().y + yOffset));
    this.myRoom.addEntity(saw);
  }
}
