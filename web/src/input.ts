import { Button } from './constants.js';
import { touchHeld, touchPressed, touchReleased } from './touchControls.js';

// Separate tracking for keyboard, gamepad, and touch so they don't interfere
const keyboardHeld = new Set<Button>();
const gamepadHeld = new Set<Button>();
const prevGamepadHeld = new Set<Button>();
const pressed = new Set<Button>();
const released = new Set<Button>();
let enabled = true;

// Keyboard: WASD + Arrows for movement, Space = jump, Ctrl/Enter = rope/fire
const keyMap = new Map<string, Button>([
  ['ArrowUp', Button.UP],
  ['ArrowDown', Button.DOWN],
  ['ArrowLeft', Button.LEFT],
  ['ArrowRight', Button.RIGHT],
  ['KeyW', Button.UP],
  ['KeyS', Button.DOWN],
  ['KeyA', Button.LEFT],
  ['KeyD', Button.RIGHT],
  ['Space', Button.JUMP],
  ['ControlLeft', Button.FIRE],
  ['ControlRight', Button.FIRE],
  ['Enter', Button.FIRE],
  ['Escape', Button.FORCE_QUIT],
  ['KeyP', Button.EXIT],
]);

// Gamepad: standard mapping (https://w3c.github.io/gamepad/#remapping)
const GAMEPAD_BUTTON_MAP: ReadonlyArray<readonly [number, Button]> = [
  [0, Button.JUMP],    // A / Cross
  [1, Button.FIRE],    // B / Circle
  [2, Button.JUMP],    // X / Square
  [3, Button.FIRE],    // Y / Triangle
  [8, Button.FORCE_QUIT], // Back / Select
  [9, Button.EXIT],    // Start
  [12, Button.UP],     // D-pad up
  [13, Button.DOWN],   // D-pad down
  [14, Button.LEFT],   // D-pad left
  [15, Button.RIGHT],  // D-pad right
] as const;

const STICK_DEADZONE = 0.3;

function isHeldByAny(button: Button): boolean {
  return keyboardHeld.has(button) || gamepadHeld.has(button) || touchHeld.has(button);
}

function pollGamepads(): void {
  const gamepads = navigator.getGamepads?.();
  if (!gamepads) return;

  gamepadHeld.clear();

  for (const gp of gamepads) {
    if (!gp || !gp.connected) continue;

    for (const [idx, button] of GAMEPAD_BUTTON_MAP) {
      if (gp.buttons[idx]?.pressed) {
        gamepadHeld.add(button);
      }
    }

    // Left stick
    const lx = gp.axes[0] ?? 0;
    const ly = gp.axes[1] ?? 0;
    if (lx < -STICK_DEADZONE) gamepadHeld.add(Button.LEFT);
    if (lx > STICK_DEADZONE) gamepadHeld.add(Button.RIGHT);
    if (ly < -STICK_DEADZONE) gamepadHeld.add(Button.UP);
    if (ly > STICK_DEADZONE) gamepadHeld.add(Button.DOWN);
  }

  // New presses: in gamepadHeld but not in prevGamepadHeld
  for (const button of gamepadHeld) {
    if (!prevGamepadHeld.has(button)) {
      pressed.add(button);
    }
  }

  // New releases: in prevGamepadHeld but not in gamepadHeld
  for (const button of prevGamepadHeld) {
    if (!gamepadHeld.has(button)) {
      released.add(button);
    }
  }

  prevGamepadHeld.clear();
  for (const b of gamepadHeld) prevGamepadHeld.add(b);
}

export const Input = {
  init(): void {
    window.addEventListener('keydown', (e: KeyboardEvent) => {
      const button = keyMap.get(e.code);
      if (button !== undefined && !keyboardHeld.has(button)) {
        e.preventDefault();
        keyboardHeld.add(button);
        pressed.add(button);
      }
    });

    window.addEventListener('keyup', (e: KeyboardEvent) => {
      const button = keyMap.get(e.code);
      if (button !== undefined) {
        e.preventDefault();
        keyboardHeld.delete(button);
        released.add(button);
      }
    });
  },

  isHeld(button: Button): boolean {
    return enabled && isHeldByAny(button);
  },

  isPressed(button: Button): boolean {
    return enabled && pressed.has(button);
  },

  isReleased(button: Button): boolean {
    return enabled && released.has(button);
  },

  update(): void {
    pressed.clear();
    released.clear();
    pollGamepads();

    // Merge touch pressed/released into the shared sets
    for (const btn of touchPressed) pressed.add(btn);
    for (const btn of touchReleased) released.add(btn);
    touchPressed.clear();
    touchReleased.clear();
  },

  enable(): void { enabled = true; },
  disable(): void { enabled = false; },
  hasTouch(): boolean {
    return ('ontouchstart' in window) || navigator.maxTouchPoints > 0;
  },
} as const;
