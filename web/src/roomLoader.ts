import { ImmutableVec2 } from './math.js';
import { Resource } from './resource.js';
import { Tile } from './tile.js';
import { Layer } from './layer.js';
import { Camera } from './camera.js';
import { Room } from './room.js';
import { Entity } from './entity.js';
import { EntityFactory } from './entityFactory.js';

export function loadRoom(levelFile: string): Room {
  const roomData = Resource.getText(levelFile);
  const data = roomData.split('\n');
  let idx = 0;

  const next = (): string => data[idx++]!.trim();
  const nextInt = (): number => parseInt(next(), 10);

  // Tile definitions
  const numTileTypes = nextInt();
  const tiles: Tile[] = [];
  const emptyTile = new Tile();

  for (let i = 0; i < numTileTypes; i++) {
    const filename = next();
    const x = nextInt();
    const y = nextInt();
    const w = nextInt();
    const h = nextInt();
    const collidable = nextInt() === 1;
    const hookable = nextInt() === 1;
    tiles.push(new Tile(Resource.getBitmap(filename), x, y, w, h, hookable, collidable));
  }

  // Background layer
  let width = nextInt();
  let height = nextInt();
  const bgLayer = new Layer(width, height);
  for (let x = 0; x < width; x++) {
    for (let y = 0; y < height; y++) {
      const t = nextInt();
      if (t !== -1) bgLayer.setTile(x, y, tiles[t]!);
    }
  }

  // Middle layer
  width = nextInt();
  height = nextInt();
  const midLayer = new Layer(width, height);
  for (let x = 0; x < width; x++) {
    for (let y = 0; y < height; y++) {
      const t = nextInt();
      midLayer.setTile(x, y, t !== -1 ? tiles[t]! : emptyTile);
    }
  }

  // Foreground layer
  width = nextInt();
  height = nextInt();
  const fgLayer = new Layer(width, height);
  for (let x = 0; x < width; x++) {
    for (let y = 0; y < height; y++) {
      const t = nextInt();
      if (t !== -1) fgLayer.setTile(x, y, tiles[t]!);
    }
  }

  const room = new Room(bgLayer, midLayer, fgLayer,
    new Camera(new ImmutableVec2(0, 0), new ImmutableVec2(midLayer.width * 10, midLayer.height * 10)));

  // Entities
  width = nextInt();
  height = nextInt();
  for (let x = 0; x < width; x++) {
    for (let y = 0; y < height; y++) {
      const entityId = nextInt();
      if (entityId !== -1) {
        const entity = EntityFactory.create(entityId);
        if (!entity) continue;
        const pos = new ImmutableVec2(10 * x + 5, 10 * (y + 1) - entity.getHalfSize().y);
        entity.setPosition(pos);
        room.addEntity(entity);
      }
    }
  }

  // Camera rects
  const rectCount = nextInt();
  for (let i = 0; i < rectCount; i++) {
    const x = nextInt();
    const y = nextInt();
    const w = nextInt();
    const h = nextInt();
    room.getCamera().addRect(x, y, w, h);
  }

  return room;
}
