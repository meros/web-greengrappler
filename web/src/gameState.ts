const PREFIX = 'gg_';
const dataMap = new Map<string, string>();

export const GameState = {
  clear(): void {
    dataMap.clear();
    // localStorage can throw (blocked storage, private mode); play on without saves.
    try {
      const keysToRemove: string[] = [];
      for (let i = 0; i < localStorage.length; i++) {
        const key = localStorage.key(i);
        if (key?.startsWith(PREFIX)) keysToRemove.push(key);
      }
      for (const key of keysToRemove) localStorage.removeItem(key);
    } catch { /* storage unavailable */ }
  },

  getInt(key: string): number {
    const val = dataMap.get(key);
    if (val === undefined) return 0;
    return parseInt(val, 10) || 0;
  },

  put(key: string, value: number): void {
    dataMap.set(key, String(value));
  },

  isSavePresent(): boolean {
    return GameState.getInt('save_present') === 1;
  },

  loadFromFile(): void {
    dataMap.clear();
    try {
      for (let i = 0; i < localStorage.length; i++) {
        const key = localStorage.key(i);
        if (key?.startsWith(PREFIX)) {
          const val = localStorage.getItem(key);
          if (val !== null) dataMap.set(key.slice(PREFIX.length), val);
        }
      }
    } catch { /* storage unavailable */ }
  },

  saveToFile(): void {
    dataMap.set('save_present', '1');
    try {
      for (const [key, val] of dataMap) {
        localStorage.setItem(PREFIX + key, val);
      }
    } catch { /* storage unavailable or full */ }
  },
} as const;
