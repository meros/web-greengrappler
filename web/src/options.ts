export const enum VibrationType {
  SIMPLE = 'SIMPLE',
  PULSATING = 'PULSATING',
}

export type ExitCallback = () => void;

let exitCallback: ExitCallback | null = null;

function isTouchDevice(): boolean {
  return ('ontouchstart' in window) || navigator.maxTouchPoints > 0;
}

export const GlobalOptions = {
  showTouchControls(): boolean { return isTouchDevice(); },
  dialogueAtTop(): boolean { return true; },
  avoidHeroAtThumbs(): boolean { return isTouchDevice(); },
  showFps(): boolean { return false; },

  vibrate(time: number, _type: VibrationType): void {
    navigator.vibrate?.(time);
  },

  setExitCallback(cb: ExitCallback): void { exitCallback = cb; },

  exit(): void {
    if (exitCallback) exitCallback();
  },
} as const;
