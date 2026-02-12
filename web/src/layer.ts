import { Tile } from './tile.js';

const TILE_SIZE = 10;
const EMPTY_TILE = new Tile();

export class Layer {
  public readonly width: number;
  public readonly height: number;
  private readonly tiles: (Tile | null)[][];
  private destroyedToTileRow = -1;

  constructor(width: number, height: number) {
    this.width = width;
    this.height = height;
    this.tiles = Array.from({ length: width }, () =>
      Array.from<Tile | null>({ length: height }).fill(null)
    );
  }

  setTile(x: number, y: number, tile: Tile): void {
    this.tiles[x]![y] = tile;
  }

  getTile(x: number, y: number): Tile {
    if (x <= this.destroyedToTileRow || y < 0 || x >= this.width || y >= this.height) {
      return EMPTY_TILE;
    }
    return this.tiles[x]?.[y] ?? EMPTY_TILE;
  }

  setDestroyedToTileRow(x: number): void {
    this.destroyedToTileRow = x;
  }

  draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, overLayers: Layer[]): void {
    const firstTileX = Math.floor(-offsetX / TILE_SIZE);
    const lastTileX = Math.floor((320 - offsetX) / TILE_SIZE);
    const firstTileY = Math.floor(-offsetY / TILE_SIZE);
    const lastTileY = Math.floor((240 - offsetY) / TILE_SIZE);

    for (let y = firstTileY; y <= lastTileY; y++) {
      for (let x = firstTileX; x <= lastTileX + 1; x++) {
        const tile = this.getTile(x, y);

        let dontDraw = false;
        for (const overLayer of overLayers) {
          const ot = overLayer.getTile(x, y);
          if (ot.isOpaque()) { dontDraw = true; break; }
        }
        if (dontDraw) continue;

        if (tile.image) {
          tile.image.draw(ctx, x * TILE_SIZE + offsetX, y * TILE_SIZE + offsetY);
        }
      }
    }
  }
}
