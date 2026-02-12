import { BitmapFont } from './media/font.js';
import { Animation } from './media/animation.js';

export interface GameImage {
  readonly width: number;
  readonly height: number;
  draw(ctx: CanvasRenderingContext2D, x: number, y: number): void;
  drawScaled(ctx: CanvasRenderingContext2D,
    dx: number, dy: number, dw: number, dh: number,
    sx: number, sy: number, sw: number, sh: number): void;
  subImage(x: number, y: number, w: number, h: number): GameImage;
  getPixel(x: number, y: number): number;
}

class CanvasGameImage implements GameImage {
  public readonly width: number;
  public readonly height: number;
  private readonly canvas: HTMLCanvasElement;
  private readonly ctx: CanvasRenderingContext2D;
  private pixelData: ImageData | null = null;

  constructor(source: HTMLCanvasElement) {
    this.canvas = source;
    this.ctx = source.getContext('2d', { willReadFrequently: true })!;
    this.width = source.width;
    this.height = source.height;
  }

  draw(ctx: CanvasRenderingContext2D, x: number, y: number): void {
    ctx.drawImage(this.canvas, x, y);
  }

  drawScaled(ctx: CanvasRenderingContext2D,
    dx: number, dy: number, dw: number, dh: number,
    sx: number, sy: number, sw: number, sh: number): void {
    ctx.drawImage(this.canvas, sx, sy, sw, sh, dx, dy, dw, dh);
  }

  subImage(x: number, y: number, w: number, h: number): GameImage {
    return new SubImage(this, x, y, w, h);
  }

  getPixel(x: number, y: number): number {
    if (!this.pixelData) {
      this.pixelData = this.ctx.getImageData(0, 0, this.width, this.height);
    }
    const idx = (y * this.width + x) * 4;
    const r = this.pixelData.data[idx]!;
    const g = this.pixelData.data[idx + 1]!;
    const b = this.pixelData.data[idx + 2]!;
    const a = this.pixelData.data[idx + 3]!;
    return (a << 24) | (r << 16) | (g << 8) | b;
  }
}

class SubImage implements GameImage {
  public readonly width: number;
  public readonly height: number;
  private readonly source: GameImage;
  private readonly sx: number;
  private readonly sy: number;
  private cached: CanvasGameImage | null = null;

  constructor(source: GameImage, sx: number, sy: number, sw: number, sh: number) {
    this.source = source;
    this.sx = sx;
    this.sy = sy;
    this.width = sw;
    this.height = sh;
  }

  draw(ctx: CanvasRenderingContext2D, x: number, y: number): void {
    this.source.drawScaled(ctx, x, y, this.width, this.height,
      this.sx, this.sy, this.width, this.height);
  }

  drawScaled(ctx: CanvasRenderingContext2D,
    dx: number, dy: number, dw: number, dh: number,
    sx: number, sy: number, sw: number, sh: number): void {
    this.source.drawScaled(ctx, dx, dy, dw, dh,
      this.sx + sx, this.sy + sy, sw, sh);
  }

  subImage(x: number, y: number, w: number, h: number): GameImage {
    return new SubImage(this.source, this.sx + x, this.sy + y, w, h);
  }

  getPixel(x: number, y: number): number {
    if (!this.cached) {
      const canvas = document.createElement('canvas');
      canvas.width = this.width;
      canvas.height = this.height;
      const ctx = canvas.getContext('2d', { willReadFrequently: true })!;
      this.draw(ctx, 0, 0);
      this.cached = new CanvasGameImage(canvas);
    }
    return this.cached.getPixel(x, y);
  }
}

function mapImagePath(path: string): string {
  return path.replace(/^data\/images\//, 'assets/images/').replace(/\.bmp$/, '.png');
}

function mapTextPath(path: string): string {
  if (path.startsWith('data/rooms/')) return path.replace(/^data\/rooms\//, 'assets/rooms/');
  if (path.startsWith('data/dialogues/')) return path.replace(/^data\/dialogues\//, 'assets/dialogues/');
  return path;
}

// Image processing: tint all visible pixels by color
function processImage(source: HTMLImageElement, tintR: number, tintG: number, tintB: number): CanvasGameImage {
  const canvas = document.createElement('canvas');
  canvas.width = source.naturalWidth;
  canvas.height = source.naturalHeight;
  const ctx = canvas.getContext('2d', { willReadFrequently: true })!;
  ctx.drawImage(source, 0, 0);

  if (tintR !== 255 || tintG !== 255 || tintB !== 255) {
    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
    const data = imageData.data;
    for (let i = 0; i < data.length; i += 4) {
      if (data[i + 3]! > 0) { // if not transparent
        data[i] = (tintR * data[i]!) / 255;
        data[i + 1] = (tintG * data[i + 1]!) / 255;
        data[i + 2] = (tintB * data[i + 2]!) / 255;
      }
    }
    ctx.putImageData(imageData, 0, 0);
  }

  return new CanvasGameImage(canvas);
}

const preloadedImages = new Map<string, HTMLImageElement>();
const preloadedTexts = new Map<string, string>();
const bitmapCache = new Map<string, GameImage>();
const animationCache = new Map<string, Animation>();
const fontCache = new Map<string, BitmapFont>();
let pendingLoads = 0;
let completedLoads = 0;

export const Resource = {
  preLoad(filename: string): void {
    const url = mapImagePath(filename);
    pendingLoads++;
    const img = new Image();
    img.onload = () => { completedLoads++; };
    img.onerror = () => { console.warn(`Failed to load: ${url}`); completedLoads++; };
    img.src = url;
    preloadedImages.set(filename, img);
  },

  preLoadText(filename: string): void {
    const url = mapTextPath(filename);
    pendingLoads++;
    fetch(url)
      .then(r => r.text())
      .then(text => { preloadedTexts.set(filename, text); completedLoads++; })
      .catch(() => { console.warn(`Failed to load: ${url}`); completedLoads++; });
  },

  preLoadSound(_sound: string): void {
    // Sounds are loaded on-demand by Sound module
    // But count as loaded immediately for preloading purposes
  },

  isDonePreloading(): boolean {
    return completedLoads >= pendingLoads;
  },

  getBitmap(filename: string, tintR = 255, tintG = 255, tintB = 255): GameImage {
    const key = `${filename}:${tintR}:${tintG}:${tintB}`;
    let cached = bitmapCache.get(key);
    if (!cached) {
      const img = preloadedImages.get(filename);
      if (!img || !img.complete) {
        // Return a 1x1 placeholder
        const c = document.createElement('canvas');
        c.width = 1; c.height = 1;
        return new CanvasGameImage(c);
      }
      cached = processImage(img, tintR, tintG, tintB);
      bitmapCache.set(key, cached);
    }
    return cached;
  },

  getText(filename: string): string {
    return preloadedTexts.get(filename) ?? '';
  },

  getAnimation(filename: string, numFrames?: number): Animation {
    const key = `${filename}:${numFrames ?? 'auto'}`;
    let cached = animationCache.get(key);
    if (!cached) {
      cached = new Animation(filename, numFrames);
      animationCache.set(key, cached);
    }
    return cached;
  },

  getFont(filename: string): BitmapFont {
    let cached = fontCache.get(filename);
    if (!cached) {
      cached = new BitmapFont(filename);
      fontCache.set(filename, cached);
    }
    return cached;
  },
};
