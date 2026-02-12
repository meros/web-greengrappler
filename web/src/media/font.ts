import type { GameImage } from '../resource.js';
import { Resource } from '../resource.js';

export class BitmapFont {
  private readonly glyphs = new Map<string, GameImage>();
  private fontHeight = 0;

  constructor(filename: string, startChar: string = ' ', endChar: string = 'z') {
    const glyphImage = Resource.getBitmap(filename);
    const chars: string[] = [];
    for (let c = startChar.charCodeAt(0); c <= endChar.charCodeAt(0); c++) {
      chars.push(String.fromCharCode(c));
    }
    this.parseGlyphs(glyphImage, chars);
  }

  private getPixel(img: GameImage, x: number, y: number): number {
    return img.getPixel(x, y);
  }

  private parseGlyphs(glyphImage: GameImage, chars: string[]): void {
    const separatingColor = this.getPixel(glyphImage, 0, 0);
    let scanLine = 0;
    let currGlyphIndex = 0;
    let lastRowHeight = 0;

    while (scanLine < glyphImage.height) {
      let x = 0;
      while (x < glyphImage.width) {
        const color = this.getPixel(glyphImage, x, scanLine);
        if (color !== separatingColor) {
          let y1 = scanLine;
          while (y1 < glyphImage.height && this.getPixel(glyphImage, x, y1) !== separatingColor) y1++;
          let x1 = x;
          while (x1 < glyphImage.width && this.getPixel(glyphImage, x1, scanLine) !== separatingColor) x1++;

          const w = x1 - x;
          const h = y1 - scanLine;
          lastRowHeight = h;

          const ch = currGlyphIndex < chars.length ? chars[currGlyphIndex]! : String.fromCharCode(200 + currGlyphIndex);
          currGlyphIndex++;
          this.glyphs.set(ch, glyphImage.subImage(x, scanLine, w, h));
          x = x1;
        }
        x += 1;
      }
      scanLine += lastRowHeight + 1;
    }

    if (!this.glyphs.has('\n') && this.glyphs.has(' ')) {
      this.glyphs.set('\n', this.glyphs.get(' ')!);
    }
    this.fontHeight = lastRowHeight;
  }

  private getGlyph(ch: string): GameImage {
    return this.glyphs.get(ch) ?? this.glyphs.get(' ')!;
  }

  private getTextWidth(text: string): number {
    let w = 0;
    for (const ch of text) w += this.getGlyph(ch).width;
    return w;
  }

  draw(ctx: CanvasRenderingContext2D, text: string, x: number, y: number): void {
    let cx = x;
    for (const ch of text) {
      const g = this.getGlyph(ch);
      g.draw(ctx, cx, y);
      cx += g.width;
    }
  }

  drawCenter(ctx: CanvasRenderingContext2D, text: string,
             x: number, y: number, w: number, h: number): void {
    const tx = x + Math.floor(w / 2 - this.getTextWidth(text) / 2);
    const ty = y + Math.floor(h / 2 - this.fontHeight / 2);
    this.draw(ctx, text, tx, ty);
  }

  drawWrap(ctx: CanvasRenderingContext2D, text: string,
           x: number, y: number, maxWidth: number, maxChars: number): void {
    let cx = x;
    let cy = y;
    let rowHeight = 0;
    let totalChars = 0;
    const words = text.split(' ');

    for (let wi = 0; wi < words.length; wi++) {
      let word = words[wi]!;
      if (wi > 0) {
        if (maxWidth > 0 && cx + this.getTextWidth(word) > maxWidth) {
          cx = x;
          cy += rowHeight;
          rowHeight = 0;
        } else {
          word = ' ' + word;
        }
      }
      for (const ch of word) {
        if (maxChars >= 0 && totalChars >= maxChars) return;
        const g = this.getGlyph(ch);
        rowHeight = Math.max(rowHeight, g.height);
        g.draw(ctx, cx, cy);
        cx += g.width;
        if (ch === '\n') { cx = x; cy += rowHeight; rowHeight = 0; }
        totalChars++;
      }
    }
  }
}
