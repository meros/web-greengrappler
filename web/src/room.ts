import { Direction } from './constants.js';
import { Vec2, ImmutableVec2, type ReadonlyVec2 } from './math.js';
import { rgb } from './utils.js';
import { Entity } from './entity.js';
import { Camera } from './camera.js';
import { Layer } from './layer.js';
import { Resource } from './resource.js';
import { Input } from './input.js';
import { Music } from './media/music.js';
import { BitmapFont } from './media/font.js';
import { Animation } from './media/animation.js';
import type { Hero } from './entities/hero.js';
import { Coin } from './entities/coin.js';
import { collides, collidesRect, type Collidable } from './collision.js';
import { ParticleSystem } from './entities/particleSystem.js';

export class Room {
  private readonly entities: Entity[] = [];
  private readonly damagableEntities: Entity[] = [];
  private readonly hookableEntities: Entity[] = [];
  private readonly staticCoins: Coin[] = [];
  private readonly entitiesToAdd: Entity[] = [];
  private hero!: Hero;

  private readonly backgroundLayer: Layer;
  private readonly middleLayer: Layer;
  private readonly foregroundLayer: Layer;
  private readonly camera: Camera;

  private readonly hookableArray: boolean[][];
  private readonly collidableArray: boolean[][];
  private destroyedToTileRow = -1;

  private frameCounter = 0;
  private heroIsDead = false;
  private isCompletedFlag = false;
  private font: BitmapFont;

  private readonly bgLayers: Layer[];
  private readonly midLayers: Layer[];
  private readonly fgLayers: Layer[] = [];

  constructor(bg: Layer, mid: Layer, fg: Layer, camera: Camera) {
    this.backgroundLayer = bg;
    this.middleLayer = mid;
    this.foregroundLayer = fg;
    this.camera = camera;
    this.bgLayers = [mid, fg];
    this.midLayers = [fg];
    this.font = Resource.getFont('data/images/font.bmp');

    this.hookableArray = Array.from({ length: this.getWidthInTiles() }, (_, x) =>
      Array.from({ length: this.getHeightInTiles() }, (_, y) =>
        mid.getTile(x, y).hookable
      )
    );
    this.collidableArray = Array.from({ length: this.getWidthInTiles() }, (_, x) =>
      Array.from({ length: this.getHeightInTiles() }, (_, y) =>
        mid.getTile(x, y).collidable
      )
    );
  }

  addEntity(entity: Entity): void {
    if ((entity as unknown as Hero).respawn !== undefined && !this.hero) {
      this.hero = entity as unknown as Hero;
    }
    entity.setRoom(this);
    this.entitiesToAdd.push(entity);
  }

  getHero(): Hero { return this.hero; }
  getCamera(): Camera { return this.camera; }
  getEntities(): Entity[] { return this.entities; }
  getTileWidth(): number { return 10; }
  getTileHeight(): number { return 10; }
  getWidthInTiles(): number { return this.middleLayer.width; }
  getHeightInTiles(): number { return this.middleLayer.height; }

  isCollidable(x: number, y: number): boolean {
    if (x < 0 || x >= this.collidableArray.length || y < 0) return false;
    const col = this.collidableArray[x];
    if (!col || y >= col.length) return false;
    if (x < this.destroyedToTileRow) return false;
    return col[y] ?? false;
  }

  isHookable(x: number, y: number): boolean {
    if (x < 0 || x >= this.hookableArray.length || y < 0) return false;
    const col = this.hookableArray[x];
    if (!col || y >= col.length) return false;
    if (x < this.destroyedToTileRow) return false;
    return col[y] ?? false;
  }

  setCollidable(x: number, y: number, v: boolean): void {
    if (x < 0 || x >= this.collidableArray.length || y < 0) return;
    const col = this.collidableArray[x];
    if (!col || y >= col.length) return;
    col[y] = v;
  }

  setHookable(x: number, y: number, v: boolean): void {
    if (x < 0 || x >= this.hookableArray.length || y < 0) return;
    const col = this.hookableArray[x];
    if (!col || y >= col.length) return;
    col[y] = v;
  }

  isCompleted(): boolean {
    if (this.isCompletedFlag && this.frameCounter > 240) {
      Music.stop();
      Input.enable();
      return true;
    }
    return false;
  }

  setCompleted(): void {
    for (const e of this.entities) e.onLevelComplete();
    this.frameCounter = 0;
    this.isCompletedFlag = true;
    this.hero.imortal();
    Input.disable();
    Music.stop();
  }

  damageDone(x: number, y: number): boolean {
    for (const e of this.damagableEntities) {
      if (collidesRect(e, x, y, x + 1, y + 1)) {
        e.onDamage();
        return true;
      }
    }
    return false;
  }

  findHookableEntity(position: ImmutableVec2): Entity | null {
    for (const e of this.hookableEntities) {
      const dx = Math.abs(e.getPosition().x - position.x);
      const dy = Math.abs(e.getPosition().y - position.y);
      if (dx < e.getHalfSize().x && dy < e.getHalfSize().y) return e;
    }
    return null;
  }

  rayCast(origin: ReadonlyVec2, direction: ReadonlyVec2, cull: boolean,
          outX: { value: number }, outY: { value: number }): boolean {
    if (new Vec2(direction).isZero()) return false;

    const tw = this.getTileWidth();
    const th = this.getTileHeight();
    let ix = Math.floor(Math.floor(origin.x) / tw);
    let iy = Math.floor(Math.floor(origin.y) / th);
    let dx = origin.x / tw - ix;
    let dy = origin.y / th - iy;
    const tx = direction.x > 0 ? 1 : 0;
    const ty = direction.y > 0 ? 1 : 0;

    let minX = 0, maxX = this.getWidthInTiles();
    let minY = 0, maxY = this.getHeightInTiles();
    if (cull) {
      minX = Math.max(minX, ix + Math.floor(Math.min(0, direction.x) / tw - 1));
      maxX = Math.min(maxX, ix + Math.floor(Math.max(0, direction.x) / tw + 2));
      minY = Math.max(minY, iy + Math.floor(Math.min(0, direction.y) / th - 1));
      maxY = Math.min(maxY, iy + Math.floor(Math.max(0, direction.y) / th + 2));
    }

    while (ix >= minX && ix < maxX && iy >= minY && iy < maxY) {
      if (this.isCollidable(ix, iy)) {
        outX.value = ix;
        outY.value = iy;
        if (cull) {
          const cx = ix * tw + tw / 2;
          const cy = iy * th + th / 2;
          const v = new Vec2(origin.x - cx, origin.y - cy);
          if (v.lengthCompare(direction) > 0) return false;
        }
        return true;
      }

      if (direction.y === 0 || (direction.x !== 0 && (tx - dx) / direction.x < (ty - dy) / direction.y)) {
        dy += direction.y * (tx - dx) / direction.x;
        if (direction.x > 0) { ix++; dx = 0; } else { ix--; dx = 1; }
      } else {
        dx += direction.x * (ty - dy) / direction.y;
        if (direction.y > 0) { iy++; dy = 0; } else { iy--; dy = 1; }
      }
    }
    return false;
  }

  broadcastButtonDown(id: number): void { for (const e of this.entities) e.onButtonDown(id); }
  broadcastButtonUp(id: number): void { for (const e of this.entities) e.onButtonUp(id); }
  broadcastStartWallOfDeath(): void { for (const e of this.entities) e.onStartWallOfDeath(); }
  broadcastBoosWallActivate(): void { for (const e of this.entities) e.onBossWallActivate(); }
  broadcastBoosWallDectivate(): void { for (const e of this.entities) e.onBossWallDeactivate(); }
  broadcastBoosFloorActivate(): void { for (const e of this.entities) e.onBossFloorActivate(); }

  destroyToTileRow(x: number): void {
    this.destroyedToTileRow = x;
    for (let y = 0; y < this.middleLayer.height; y++) {
      if (this.middleLayer.getTile(x, y).collidable) {
        const ps = new ParticleSystem(
          Resource.getAnimation('data/images/debris.bmp', 4),
          20, 30, 10, 15, 40, 3, new ImmutableVec2(0, -50), 5,
        );
        ps.setPosition(new ImmutableVec2(x * this.getTileWidth() + this.getTileWidth() * 0.75,
          y * this.getTileHeight() + this.getTileHeight() * 0.75));
        this.addEntity(ps);
      }
    }
    this.middleLayer.setDestroyedToTileRow(x);
    this.foregroundLayer.setDestroyedToTileRow(x);
  }

  onDraw(ctx: CanvasRenderingContext2D): void {
    const ox = this.camera.getOffsetX();
    const oy = this.camera.getOffsetY();

    if (this.hero.isDead()) {
      const fc = this.frameCounter;
      ctx.fillStyle = (fc === 0 || fc === 5 || fc === 10) ? rgb(231, 215, 156) : rgb(57, 56, 41);
      ctx.fillRect(0, 0, 320, 240);
      if (fc < 60) this.hero.draw(ctx, ox, oy, 0);
      return;
    }

    this.backgroundLayer.draw(ctx, ox, oy, this.bgLayers);
    this.middleLayer.draw(ctx, ox, oy, this.midLayers);

    for (const e of this.entities) e.draw(ctx, ox, oy, 0);

    this.foregroundLayer.draw(ctx, ox, oy, this.fgLayers);

    if (this.isCompletedFlag) {
      this.font.drawCenter(ctx, 'LEVEL COMPLETED!', 0, 0, 320, 240);
    }
  }

  onLogic(): void {
    this.frameCounter++;

    if (this.entitiesToAdd.length > 0) {
      for (const e of this.entitiesToAdd) {
        if (e.isDamagable()) this.damagableEntities.push(e);
        if (e.isHookable()) this.hookableEntities.push(e);
        if (e instanceof Coin && !e.isDynamic()) this.staticCoins.push(e);
      }
      this.entities.push(...this.entitiesToAdd);
      this.entities.sort((a, b) => {
        if (a.getLayer() !== b.getLayer()) return a.getLayer() - b.getLayer();
        return a.constructor.name.localeCompare(b.constructor.name);
      });
      this.entitiesToAdd.length = 0;
    }

    if (this.isCompletedFlag && this.frameCounter === 60)
      Music.playSong('data/music/level_completed.xm');

    if (this.hero.isDead() && !this.heroIsDead) {
      this.frameCounter = 0;
      this.heroIsDead = true;
      return;
    }

    if (this.hero.isDead() && this.frameCounter > 120) {
      this.hero.respawn();
      for (const e of this.entities) e.onRespawn();
      this.camera.centerToHero(this.hero);
      this.camera.onRespawn();
      this.heroIsDead = false;
    }

    if (this.hero.isDead()) return;

    for (let i = this.entities.length - 1; i >= 0; i--) {
      if (this.entities[i]!.isRemoved()) {
        const e = this.entities[i]!;
        this.entities.splice(i, 1);
        const hi = this.hookableEntities.indexOf(e);
        if (hi >= 0) this.hookableEntities.splice(hi, 1);
        const di = this.damagableEntities.indexOf(e);
        if (di >= 0) this.damagableEntities.splice(di, 1);
      }
    }

    for (let i = 0; i < this.entities.length; i++) {
      const e = this.entities[i]!;
      if (this.hero.isDead()) break;
      if (e !== (this.hero as unknown as Entity)) e.update();
    }

    this.hero.update();

    // Static coin collision
    for (let i = this.staticCoins.length - 1; i >= 0; i--) {
      const coin = this.staticCoins[i]!;
      if (coin.isRemoved()) {
        this.staticCoins.splice(i, 1);
        continue;
      }
      if (coin.Collides(this.hero)) {
        coin.setCollides();
      }
    }

    // Check out-of-bounds
    const maxW = this.getWidthInTiles() * this.getTileWidth();
    const maxH = this.getHeightInTiles() * this.getTileHeight();
    for (const e of this.entities) {
      const p = e.getPosition();
      const hs = e.getHalfSize();
      if (p.x - hs.x > maxW || p.y - hs.y > maxH || p.x + hs.x < 0 || p.y + hs.y < 0) {
        e.remove();
      }
    }

    this.camera.update(this.hero);
  }
}
