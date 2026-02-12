interface Song {
  play(): void;
  stop(): void;
}

let currentSong: Song | null = null;
const songStack: Song[] = [];

function mapPath(path: string): string {
  // "data/music/olof12.xm" -> "assets/music/olof12.ogg"
  const name = path.replace(/^data\/music\//, '').replace(/\.xm$/, '');
  return `assets/music/${name}.ogg`;
}

function createSong(url: string): Song {
  const audio = new Audio(url);
  audio.loop = true;
  audio.volume = 0.7;
  return {
    play(): void {
      audio.currentTime = 0;
      audio.play().catch(() => { /* user interaction required */ });
    },
    stop(): void {
      audio.pause();
      audio.currentTime = 0;
    },
  };
}

export const Music = {
  playSong(path: string): void {
    Music.stop();
    currentSong = createSong(mapPath(path));
    currentSong.play();
  },

  play(): void {
    currentSong?.play();
  },

  stop(): void {
    currentSong?.stop();
  },

  pushSong(): void {
    Music.stop();
    if (currentSong) songStack.push(currentSong);
  },

  popSong(): void {
    Music.stop();
    if (songStack.length > 0) {
      currentSong = songStack.pop()!;
    }
    Music.play();
  },

  update(): void {
    // No-op for OGG playback (HTML audio handles looping)
  },
} as const;
