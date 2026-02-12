const PREFIX = 'gg_';
const dataMap = new Map<string, string>();

export const GameState = {
  clear(): void {
    dataMap.clear();
    const keysToRemove: string[] = [];
    for (let i = 0; i < localStorage.length; i++) {
      const key = localStorage.key(i);
      if (key?.startsWith(PREFIX)) keysToRemove.push(key);
    }
    for (const key of keysToRemove) localStorage.removeItem(key);
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
    for (let i = 0; i < localStorage.length; i++) {
      const key = localStorage.key(i);
      if (key?.startsWith(PREFIX)) {
        const val = localStorage.getItem(key);
        if (val !== null) dataMap.set(key.slice(PREFIX.length), val);
      }
    }
  },

  saveToFile(): void {
    dataMap.set('save_present', '1');
    for (const [key, val] of dataMap) {
      localStorage.setItem(PREFIX + key, val);
    }
  },
} as const;
