import { Button } from './constants.js';

const held = new Set<Button>();
const pressed = new Set<Button>();
const released = new Set<Button>();
let enabled = true;

const keyMap = new Map<string, Button>([
  ['ArrowUp', Button.UP],
  ['ArrowDown', Button.DOWN],
  ['ArrowLeft', Button.LEFT],
  ['ArrowRight', Button.RIGHT],
  ['Enter', Button.FIRE],
  ['KeyZ', Button.FIRE],
  ['Escape', Button.FORCE_QUIT],
  ['KeyA', Button.JUMP],
  ['KeyP', Button.EXIT],
  ['Space', Button.JUMP],
]);

function onButtonDown(button: Button): void {
  pressed.add(button);
  held.add(button);
}

function onButtonUp(button: Button): void {
  held.delete(button);
  released.add(button);
}

export const Input = {
  init(): void {
    window.addEventListener('keydown', (e: KeyboardEvent) => {
      const button = keyMap.get(e.code);
      if (button !== undefined && !held.has(button)) {
        e.preventDefault();
        onButtonDown(button);
      }
    });

    window.addEventListener('keyup', (e: KeyboardEvent) => {
      const button = keyMap.get(e.code);
      if (button !== undefined) {
        e.preventDefault();
        onButtonUp(button);
      }
    });
  },

  isHeld(button: Button): boolean {
    return enabled && held.has(button);
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
  },

  enable(): void { enabled = true; },
  disable(): void { enabled = false; },
  hasTouch(): boolean { return false; },
} as const;
