import { TICKS_PER_SECOND, Direction } from './constants.js';
import { Vec2, ImmutableVec2, type ReadonlyVec2 } from './math.js';
import { collides, type Collidable } from './collision.js';
import type { Room } from './room.js';

export abstract class Entity implements Collidable {
  protected myFrameCounter = 0;
  protected myRemoved = false;
  protected myRoom!: Room;
  private readonly position = new Vec2();
  protected readonly myVelocity = new Vec2();
  private size = new ImmutableVec2();
  private halfSize = new ImmutableVec2();
  private readonly moveResult = new Set<Direction>();

  abstract getLayer(): number;

  getPosition(): Vec2 { return this.position; }
  getVelocity(): Vec2 { return this.myVelocity; }
  getSize(): ImmutableVec2 { return this.size; }
  getHalfSize(): ImmutableVec2 { return this.halfSize; }
  getDrawPositionX(): number { return Math.round(this.position.x); }
  getDrawPositionY(): number { return Math.round(this.position.y); }

  setPosition(pos: ReadonlyVec2): void { this.position.set(pos); }
  setVelocity(vel: ReadonlyVec2): void { this.myVelocity.set(vel); }

  setSize(s: ImmutableVec2): void {
    this.size = s;
    this.halfSize = s.multiply(0.5);
  }

  setRoom(room: Room): void { this.myRoom = room; }
  getRoom(): Room { return this.myRoom; }

  isRemoved(): boolean { return this.myRemoved; }
  remove(): void { this.myRemoved = true; }

  isDamagable(): boolean { return false; }
  isHookable(): boolean { return false; }

  getCollideTop(): number { return this.position.y - this.halfSize.y; }
  getCollideLeft(): number { return this.position.x - this.halfSize.x; }
  getCollideBottom(): number { return this.position.y + this.halfSize.y; }
  getCollideRight(): number { return this.position.x + this.halfSize.x; }

  Collides(other: Collidable): boolean { return collides(this, other); }

  draw(_ctx: CanvasRenderingContext2D, _offsetX: number, _offsetY: number, _layer: number): void {}

  update(): void { this.myFrameCounter++; }

  moveWithCollision(dx?: number, dy?: number): Set<Direction> {
    let deltaX = dx ?? this.myVelocity.x / TICKS_PER_SECOND;
    let deltaY = dy ?? this.myVelocity.y / TICKS_PER_SECOND;

    const substeps = Math.max(1, Math.ceil((Math.abs(deltaX) + Math.abs(deltaY)) * 0.2));
    deltaX /= substeps;
    deltaY /= substeps;

    this.moveResult.clear();
    const hs = this.halfSize;
    const pos = this.position;
    const room = this.myRoom;
    const tw = room.getTileWidth();
    const th = room.getTileHeight();

    for (let i = 0; i < substeps; i++) {
      // X movement
      pos.x += deltaX;
      const x1 = Math.floor((pos.x - hs.x) / tw);
      const x2 = Math.floor((pos.x + hs.x) / tw);
      const y1n = Math.floor((pos.y - hs.y + 0.01) / th);
      const y2n = Math.floor((pos.y + hs.y - 0.01) / th);

      if (deltaX > 0) {
        for (let y = y1n; y <= y2n; y++) {
          if (room.isCollidable(x2, y)) {
            deltaX = 0;
            this.moveResult.add(Direction.RIGHT);
            pos.x = x2 * tw - hs.x;
            break;
          }
        }
      } else if (deltaX < 0) {
        for (let y = y1n; y <= y2n; y++) {
          if (room.isCollidable(x1, y)) {
            deltaX = 0;
            this.moveResult.add(Direction.LEFT);
            pos.x = (x1 + 1) * tw + hs.x;
            break;
          }
        }
      }

      // Y movement
      pos.y += deltaY;
      const y1 = Math.floor((pos.y - hs.y) / th);
      const y2 = Math.floor((pos.y + hs.y) / th);
      const x1n = Math.floor((pos.x - hs.x + 0.01) / tw);
      const x2n = Math.floor((pos.x + hs.x - 0.01) / tw);

      if (deltaY > 0) {
        for (let x = x1n; x <= x2n; x++) {
          if (room.isCollidable(x, y2)) {
            deltaY = 0;
            this.moveResult.add(Direction.DOWN);
            pos.y = y2 * th - hs.y;
            break;
          }
        }
      } else if (deltaY < 0) {
        for (let x = x1n; x <= x2n; x++) {
          if (room.isCollidable(x, y1)) {
            deltaY = 0;
            this.moveResult.add(Direction.UP);
            pos.y = (y1 + 1) * th + hs.y;
            break;
          }
        }
      }
    }
    return this.moveResult;
  }

  // Override hooks
  onBossFloorActivate(): void {}
  onBossWallActivate(): void {}
  onBossWallDeactivate(): void {}
  onButtonDown(_id: number): void {}
  onButtonUp(_id: number): void {}
  onDamage(): void {}
  onLevelComplete(): void {}
  onRespawn(): void {}
  onStartWallOfDeath(): void {}
}
