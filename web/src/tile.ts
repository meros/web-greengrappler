import type { GameImage } from './resource.js';

export class Tile {
  public readonly image: GameImage | null;
  public readonly hookable: boolean;
  public readonly collidable: boolean;

  constructor();
  constructor(tilemap: GameImage, x: number, y: number, w: number, h: number,
              hookable: boolean, collidable: boolean);
  constructor(tilemap?: GameImage, x?: number, y?: number, w?: number, h?: number,
              hookable?: boolean, collidable?: boolean) {
    if (tilemap && x !== undefined && y !== undefined && w !== undefined && h !== undefined) {
      this.image = tilemap.subImage(x, y, w, h);
      this.hookable = hookable ?? false;
      this.collidable = collidable ?? false;
    } else {
      this.image = null;
      this.hookable = false;
      this.collidable = false;
    }
  }

  isOpaque(): boolean { return this.image !== null; }
}
