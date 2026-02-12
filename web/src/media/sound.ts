let audioCtx: AudioContext | null = null;
let masterGain: GainNode | null = null;
const bufferCache = new Map<string, AudioBuffer>();
const loadingPromises = new Map<string, Promise<AudioBuffer | null>>();

const SFX_VOLUME = 0.5;

function getContext(): AudioContext {
  if (!audioCtx) {
    audioCtx = new AudioContext();
    masterGain = audioCtx.createGain();
    masterGain.gain.value = SFX_VOLUME;
    masterGain.connect(audioCtx.destination);
  }
  return audioCtx;
}

function mapPath(path: string): string {
  // "data/sounds/jump" -> "assets/sounds/jump.mp3"
  const name = path.replace(/^data\/sounds\//, '');
  return `assets/sounds/${name}.mp3`;
}

async function loadBuffer(url: string): Promise<AudioBuffer | null> {
  try {
    const resp = await fetch(url);
    const data = await resp.arrayBuffer();
    return await getContext().decodeAudioData(data);
  } catch {
    console.warn(`Failed to load sound: ${url}`);
    return null;
  }
}

export const Sound = {
  preload(path: string): void {
    const url = mapPath(path);
    if (!loadingPromises.has(url)) {
      loadingPromises.set(url, loadBuffer(url).then(buf => {
        if (buf) bufferCache.set(url, buf);
        return buf;
      }));
    }
  },

  playSample(path: string): void {
    const url = mapPath(path);
    const buffer = bufferCache.get(url);
    if (!buffer) return;
    const ctx = getContext();
    const source = ctx.createBufferSource();
    source.buffer = buffer;
    source.connect(masterGain!);
    source.start();
  },
  resumeContext(): void {
    if (audioCtx && audioCtx.state === 'suspended') {
      audioCtx.resume().catch(() => {});
    }
  },
} as const;
