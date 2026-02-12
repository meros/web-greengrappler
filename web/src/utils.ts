import { ImmutableVec2 } from './math.js';

export function lerp(min: number, max: number, ratio: number): number {
  if (ratio < 0) return min;
  if (ratio > 1) return max;
  return min + (max - min) * ratio;
}

export function lerpInt(min: number, max: number, ratio: number): number {
  return Math.floor(lerp(min, max, ratio));
}

export function sincos(d: number): ImmutableVec2 {
  return new ImmutableVec2(Math.cos(d), Math.sin(d));
}

export function rgb(r: number, g: number, b: number): string {
  return `rgb(${r},${g},${b})`;
}

export function selectFeedback(): void {
  // No-op on web (was vibration on mobile)
}

export function selectVibration(): void {
  // No-op on web
}
