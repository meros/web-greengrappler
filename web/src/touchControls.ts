import { Button, TICKS_PER_SECOND } from './constants.js';
import { Resource } from './resource.js';

// Touch zones from original Android Java code (in 960x720 overlay space).
// The controls.png image is 960x720 = 3x the 320x240 game canvas.
const OVERLAY_SCALE = 3;

interface TouchZone {
  readonly button: Button;
  readonly x: number;
  readonly y: number;
  readonly w: number;
  readonly h: number;
}

const TOUCH_ZONES: readonly TouchZone[] = [
  { button: Button.FIRE,  x: 788, y: 465, w: 172, h: 234 },
  { button: Button.JUMP,  x: 616, y: 465, w: 172, h: 234 },
  { button: Button.LEFT,  x: -35, y: 355, w: 160, h: 400 },
  { button: Button.RIGHT, x: 205, y: 355, w: 160, h: 400 },
  { button: Button.UP,    x: -35, y: 355, w: 400, h: 160 },
  { button: Button.DOWN,  x: -35, y: 595, w: 400, h: 160 },
  { button: Button.EXIT,  x: 860, y: 0,   w: 100, h: 100 },
];

// Help text labels and their positions (in canvas coords 320x240)
const HELP_LABELS: readonly { text: string; x: number; y: number }[] = [
  { text: 'MOVE', x: 38, y: 138 },
  { text: 'JUMP', x: 197, y: 175 },
  { text: 'ROPE', x: 278, y: 155 },
];

// Fade timing
const CONTROLS_ALPHA = 0.6;
const FADE_HOLD_TICKS = 2 * TICKS_PER_SECOND;
const FADE_OUT_TICKS = 3 * TICKS_PER_SECOND;
const HELP_SHOW_TICKS = 3 * TICKS_PER_SECOND;
const HELP_FADE_TICKS = 2 * TICKS_PER_SECOND;
// On load, briefly show the controls so help text makes sense
const INITIAL_SHOW_TICKS = 3 * TICKS_PER_SECOND;
const INITIAL_FADE_TICKS = 2 * TICKS_PER_SECOND;

// State
let overlayImage: HTMLImageElement | null = null;
let canvas: HTMLCanvasElement | null = null;
let overlayAlpha = 0;
let fadeState: 'hidden' | 'visible' | 'holding' | 'fading' | 'initial_show' | 'initial_fade' = 'hidden';
let fadeTimer = 0;
let helpTimer = 0;
let touchActive = false;

// Button state for Input to consume
export const touchHeld = new Set<Button>();
export const touchPressed = new Set<Button>();
export const touchReleased = new Set<Button>();

function screenToCanvas(clientX: number, clientY: number): [number, number] {
  if (!canvas) return [0, 0];
  const rect = canvas.getBoundingClientRect();
  return [
    (clientX - rect.left) / rect.width * 320,
    (clientY - rect.top) / rect.height * 240,
  ];
}

/** Hit-test a point in canvas coordinates (320x240) against touch zones. */
export function hitTest(canvasX: number, canvasY: number): Button[] {
  const ox = canvasX * OVERLAY_SCALE;
  const oy = canvasY * OVERLAY_SCALE;
  const buttons: Button[] = [];
  for (const zone of TOUCH_ZONES) {
    if (ox >= zone.x && ox <= zone.x + zone.w &&
        oy >= zone.y && oy <= zone.y + zone.h) {
      buttons.push(zone.button);
    }
  }
  return buttons;
}

function handleTouchEvent(e: TouchEvent): void {
  e.preventDefault();
  touchActive = true;

  // Show overlay when touching
  if (e.touches.length > 0) {
    overlayAlpha = CONTROLS_ALPHA;
    fadeState = 'visible';
    fadeTimer = 0;
  }

  const oldHeld = new Set(touchHeld);
  touchHeld.clear();

  for (let i = 0; i < e.touches.length; i++) {
    const touch = e.touches[i]!;
    const [cx, cy] = screenToCanvas(touch.clientX, touch.clientY);
    for (const btn of hitTest(cx, cy)) {
      touchHeld.add(btn);
    }
  }

  // Compute pressed (newly held)
  for (const btn of touchHeld) {
    if (!oldHeld.has(btn)) touchPressed.add(btn);
  }
  // Compute released (no longer held)
  for (const btn of oldHeld) {
    if (!touchHeld.has(btn)) touchReleased.add(btn);
  }

  // Start fade when all touches end
  if (e.touches.length === 0 && (fadeState === 'visible' || fadeState === 'initial_show')) {
    fadeState = 'holding';
    fadeTimer = FADE_HOLD_TICKS;
  }
}

export const TouchControls = {
  init(gameCanvas: HTMLCanvasElement): void {
    canvas = gameCanvas;

    // Load overlay image
    overlayImage = new Image();
    overlayImage.src = 'assets/images/controls.png';

    // Touch event listeners
    canvas.addEventListener('touchstart', handleTouchEvent, { passive: false });
    canvas.addEventListener('touchmove', handleTouchEvent, { passive: false });
    canvas.addEventListener('touchend', handleTouchEvent, { passive: false });
    canvas.addEventListener('touchcancel', handleTouchEvent, { passive: false });

    // On touch-capable devices, show controls + help text on load
    if ('ontouchstart' in window || navigator.maxTouchPoints > 0) {
      touchActive = true;
      fadeState = 'initial_show';
      fadeTimer = INITIAL_SHOW_TICKS;
      overlayAlpha = CONTROLS_ALPHA;
      helpTimer = HELP_SHOW_TICKS + HELP_FADE_TICKS;
    }
  },

  update(): void {
    // Update help text timer
    if (helpTimer > 0) helpTimer--;

    // Update controls fade
    switch (fadeState) {
      case 'initial_show':
        fadeTimer--;
        if (fadeTimer <= 0) {
          fadeState = 'initial_fade';
          fadeTimer = INITIAL_FADE_TICKS;
        }
        break;
      case 'initial_fade':
        fadeTimer--;
        overlayAlpha = CONTROLS_ALPHA * (fadeTimer / INITIAL_FADE_TICKS);
        if (fadeTimer <= 0) {
          fadeState = 'hidden';
          overlayAlpha = 0;
        }
        break;
      case 'holding':
        fadeTimer--;
        if (fadeTimer <= 0) {
          fadeState = 'fading';
          fadeTimer = FADE_OUT_TICKS;
        }
        break;
      case 'fading':
        fadeTimer--;
        overlayAlpha = CONTROLS_ALPHA * (fadeTimer / FADE_OUT_TICKS);
        if (fadeTimer <= 0) {
          fadeState = 'hidden';
          overlayAlpha = 0;
        }
        break;
    }
  },

  draw(ctx: CanvasRenderingContext2D): void {
    // Draw controls overlay
    if (overlayImage?.complete && overlayAlpha > 0) {
      ctx.save();
      ctx.globalAlpha = overlayAlpha;
      ctx.drawImage(overlayImage, 0, 0, 960, 720, 0, 0, 320, 240);
      ctx.restore();
    }

    // Draw help text
    if (helpTimer > 0 && touchActive) {
      let alpha = 1;
      if (helpTimer < HELP_FADE_TICKS) {
        alpha = helpTimer / HELP_FADE_TICKS;
      }
      ctx.save();
      ctx.globalAlpha = alpha;
      const font = Resource.getFont('data/images/font.bmp');
      for (const label of HELP_LABELS) {
        font.draw(ctx, label.text, label.x, label.y);
      }
      ctx.restore();
    }
  },

  hasTouch(): boolean {
    return touchActive || ('ontouchstart' in window) || navigator.maxTouchPoints > 0;
  },

  isActive(): boolean {
    return touchActive;
  },
};
