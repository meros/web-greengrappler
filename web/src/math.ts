const EPSILON = 0.0000001;

export interface ReadonlyVec2 {
  readonly x: number;
  readonly y: number;
  length(): number;
  lengthCompare(val: number | ReadonlyVec2): number;
  isZero(): boolean;
  dot(other: ReadonlyVec2): number;
}

export class Vec2 implements ReadonlyVec2 {
  public x: number;
  public y: number;

  constructor();
  constructor(other: ReadonlyVec2);
  constructor(x: number, y: number);
  constructor(xOrOther?: number | ReadonlyVec2, y?: number) {
    if (xOrOther === undefined) {
      this.x = 0; this.y = 0;
    } else if (typeof xOrOther === 'object') {
      this.x = xOrOther.x; this.y = xOrOther.y;
    } else {
      this.x = xOrOther; this.y = y ?? 0;
    }
  }

  set(other: ReadonlyVec2): this;
  set(x: number, y: number): this;
  set(xOrOther: number | ReadonlyVec2, y?: number): this {
    if (typeof xOrOther === 'object') {
      this.x = xOrOther.x; this.y = xOrOther.y;
    } else {
      this.x = xOrOther; this.y = y!;
    }
    return this;
  }

  add(other: ReadonlyVec2): this;
  add(x: number, y: number): this;
  add(xOrOther: number | ReadonlyVec2, y?: number): this {
    if (typeof xOrOther === 'object') {
      this.x += xOrOther.x; this.y += xOrOther.y;
    } else {
      this.x += xOrOther; this.y += y ?? 0;
    }
    return this;
  }

  subtract(other: ReadonlyVec2): this;
  subtract(x: number, y: number): this;
  subtract(xOrOther: number | ReadonlyVec2, y?: number): this {
    if (typeof xOrOther === 'object') {
      this.x -= xOrOther.x; this.y -= xOrOther.y;
    } else {
      this.x -= xOrOther; this.y -= y ?? 0;
    }
    return this;
  }

  multiply(factor: number): this { this.x *= factor; this.y *= factor; return this; }
  divide(d: number): this { this.x /= d; this.y /= d; return this; }

  normalize(): this {
    const len = this.length();
    if (len < EPSILON) return this;
    return this.divide(len);
  }

  length(): number { return Math.sqrt(this.x * this.x + this.y * this.y); }

  lengthCompare(val: number | ReadonlyVec2): number {
    const selfSq = this.x * this.x + this.y * this.y;
    if (typeof val === 'number') return selfSq - val * val;
    return selfSq - (val.x * val.x + val.y * val.y);
  }

  isZero(): boolean { return this.length() < EPSILON; }
  dot(other: ReadonlyVec2): number { return this.x * other.x + this.y * other.y; }

  toImmutable(): ImmutableVec2 { return new ImmutableVec2(this.x, this.y); }
}

export class ImmutableVec2 implements ReadonlyVec2 {
  public readonly x: number;
  public readonly y: number;

  constructor();
  constructor(other: ReadonlyVec2);
  constructor(x: number, y: number);
  constructor(xOrOther?: number | ReadonlyVec2, y?: number) {
    if (xOrOther === undefined) {
      this.x = 0; this.y = 0;
    } else if (typeof xOrOther === 'object') {
      this.x = xOrOther.x; this.y = xOrOther.y;
    } else {
      this.x = xOrOther; this.y = y ?? 0;
    }
  }

  add(other: ReadonlyVec2): ImmutableVec2 {
    return new ImmutableVec2(this.x + other.x, this.y + other.y);
  }
  subtract(other: ReadonlyVec2): ImmutableVec2 {
    return new ImmutableVec2(this.x - other.x, this.y - other.y);
  }
  multiply(f: number): ImmutableVec2 { return new ImmutableVec2(this.x * f, this.y * f); }
  divide(d: number): ImmutableVec2 { return new ImmutableVec2(this.x / d, this.y / d); }

  normalize(): ImmutableVec2 {
    const len = this.length();
    if (len < EPSILON) return this;
    return this.divide(len);
  }

  length(): number { return Math.sqrt(this.x * this.x + this.y * this.y); }

  lengthCompare(val: number | ReadonlyVec2): number {
    const selfSq = this.x * this.x + this.y * this.y;
    if (typeof val === 'number') return selfSq - val * val;
    return selfSq - (val.x * val.x + val.y * val.y);
  }

  isZero(): boolean { return this.length() < EPSILON; }
  dot(other: ReadonlyVec2): number { return this.x * other.x + this.y * other.y; }
}

export const ZERO_VEC: ReadonlyVec2 = new ImmutableVec2(0, 0);
