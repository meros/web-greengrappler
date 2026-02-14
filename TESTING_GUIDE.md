# Green Grappler Wii Build - Testing Guide

**Status:** ✅ Binary rebuilt with all crash-fix improvements
**Build Time:** 2026-02-14 19:44 CET
**Ready to Test:** YES

---

## What Was Fixed in This Build

### 1. **Flushed Debug Output** ✅ CRITICAL FIX
**Problem:** Debug messages might not appear if stderr isn't flushed before a crash
**Solution:** Added `std::fflush(stderr)` after each fprintf statement
- Messages now appear immediately, even if the program crashes
- This is essential for identifying the exact crash point

### 2. **Renderer Fallback** ✅
**Problem:** SDL_RENDERER_ACCELERATED might not be available on Wii
**Solution:** Try hardware rendering first, fall back to software rendering
```cpp
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
if (!renderer) {
    renderer = SDL_CreateRenderer(window, -1, 0);  // Software fallback
}
```

### 3. **Proper Wii Display Mode** ✅
**Problem:** Desktop window size (640x480) might not work on Wii
**Solution:** Use fullscreen at native resolution (320x240) on Wii
```cpp
#ifdef HW_RVL
    const int WIDTH = SCREEN_WIDTH;        // 320
    const int HEIGHT = SCREEN_HEIGHT;      // 240
    const Uint32 FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
#else
    const int WIDTH = SCREEN_WIDTH * WINDOW_SCALE;  // 640
    const int HEIGHT = SCREEN_HEIGHT * WINDOW_SCALE; // 480
    const Uint32 FLAGS = SDL_WINDOW_SHOWN;
#endif
```

### 4. **Comprehensive Debug Output** ✅
Now prints at each initialization step with automatic flushing:
```
DEBUG: Initializing FAT...
DEBUG: FAT initialized
DEBUG: Initializing VIDEO...
DEBUG: VIDEO initialized
DEBUG: Initializing WPAD...
DEBUG: WPAD initialized
DEBUG: Initializing SDL...
DEBUG: SDL initialized
DEBUG: Creating window (320x240)...
DEBUG: Window created (320x240)
DEBUG: Creating renderer...
DEBUG: Renderer created
DEBUG: Initializing Resource manager...
DEBUG: Resource manager initialized
DEBUG: Initializing Sound...
DEBUG: Sound initialized
DEBUG: Initializing Input...
DEBUG: Input initialized
DEBUG: Starting asset preload...
DEBUG: Loading 32 images...
DEBUG: Loaded 32 images
DEBUG: Loading 17 sounds...
DEBUG: Loaded 17 sounds
DEBUG: Loading 4 text files...
DEBUG: Loaded 4 text files
DEBUG: Assets preloaded
```

---

## How to Test

### Quick Start

```bash
# Option 1: Use the provided test script
./run-dolphin.sh

# Option 2: Direct Dolphin launch (if installed)
dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol

# Option 3: Via Nix
nix-shell -p dolphin-emu --run "dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol"
```

### Viewing Debug Output in Dolphin

**Method 1: Built-in Log Window (Recommended)**
1. Launch Dolphin with the game
2. Press **Shift+L** to show the log window
3. Watch for the "DEBUG:" messages
4. Note which message appears last before the crash

**Method 2: Capture to File**
```bash
# Run Dolphin and capture stderr
nix-shell -p dolphin-emu --run "dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol 2>&1" | tee dolphin-output.log

# Then view the log
cat dolphin-output.log | grep DEBUG
```

**Method 3: Search for Errors**
Look for any error messages after the last DEBUG message:
```bash
cat dolphin-output.log | tail -20
```

---

## Expected Behavior

### If Everything Works ✅
You should see:
1. Green background (Wii video initialization)
2. All DEBUG messages appear in the log
3. "DEBUG: Assets preloaded" is the last message
4. Splash screen appears with animation
5. Title screen displays with menu options
6. Game responds to controller input

### If It Crashes 🔴

**Use the debug output to identify where:**

| Last Debug Message | Issue | Next Step |
|---|---|---|
| `Initializing FAT...` | FAT initialization failed | Check SD card setup |
| `FAT initialized` | FAT worked, VIDEO problem | Check VIDEO_Init |
| `VIDEO initialized` | Video worked, WPAD problem | Check WPAD_Init |
| `WPAD initialized` | WPAD worked, SDL problem | Check SDL_Init error message |
| `Initializing SDL...` (no completion) | SDL_Init crashed or failed | Look for SDL error message |
| `SDL initialized` | SDL worked, Window problem | Check window dimensions |
| `Creating window...` (no completion) | Window creation crashed | Check screen resolution |
| `Window created` | Window worked, Renderer problem | Renderer fallback should handle |
| `Creating renderer...` (no completion) | Both renderers failed | Check SDL_GetError message |
| `Renderer created` | Renderer worked, Resource problem | Check Resource::init |
| `Resource manager initialized` | Resources worked, Sound problem | Check Sound::init |
| `Sound initialized` | Sound worked, Input problem | Check Input::init |
| `Input initialized` | Input worked, Asset problem | Check which asset fails |
| `Starting asset preload...` | Preload started | Which asset fails? |
| `Loading 32 images...` | Images started loading | Check image count vs actual |
| `Loaded 32 images` | Images ok, sounds problem | Check sound count |
| `Loading 17 sounds...` | Sounds started | Check sound files |
| `Loaded 17 sounds` | Sounds ok, text problem | Check text files |
| `Loading 4 text files...` | Text started | Check which text fails |
| `Loaded 4 text files` | All loaded! | Game should start |

---

## Troubleshooting Guide

### Symptom: Green screen only (no crash reported)
The game might be running silently without rendering. This would mean all initialization passed but the game loop isn't producing output.
- Check if Dolphin is actually rendering
- Try pressing HOME to quit
- If the emulator window closes, the game is responding but not rendering

### Symptom: Renderer message says "Hardware renderer failed"
This is NORMAL and expected. The fallback to software rendering should work fine.
- Check the next message to see if software renderer was created
- Software rendering is slower but should work on all Wii systems

### Symptom: Specific error message with SDL
If you see `SDL_Init failed: ...` or similar:
1. Note the exact error message
2. Search SDL2 documentation for that error
3. Common issues:
   - No video driver available
   - Audio subsystem not available
   - Gamepad subsystem issues

### Symptom: Asset loading fails
If it stops at `Loading X ...`:
1. Check that `dolphin-game/apps/greengrappler/data/` exists
2. Verify asset files exist (images/, sounds/, music/, rooms/, dialogues/)
3. Check asset formats are correct (PNG for images, MP3/OGG for sounds)

---

## What to Report

If the game doesn't work, please provide:

1. **The last "DEBUG:" message printed**
   - This identifies exactly where it fails

2. **Any error messages** that appear after the last DEBUG message
   - These explain WHY it failed

3. **Whether hardware renderer was tried**
   - Look for "Hardware renderer failed" message

4. **Dolphin version** you're using
   - Some versions may have issues

5. **Your system** (optional)
   - OS and CPU help with troubleshooting

---

## Binary Information

**File:** `dolphin-game/apps/greengrappler/boot.dol`
**Size:** 3.1 MB
**Architecture:** PowerPC 750 (Gekko) - 32-bit big-endian
**Entry Point:** 0x80003f00
**No Undefined Symbols:** ✓

The binary is fully linked and ready to run on Wii hardware or in Dolphin.

---

## Next Steps

1. **Test in Dolphin** (this is critical)
   - Run: `./run-dolphin.sh`
   - Watch the debug output
   - Report the last message + any errors

2. **Expected Success Indicators**
   - All DEBUG messages appear
   - Splash screen shows
   - Title screen displays
   - Game starts when you press a button

3. **If it Still Fails**
   - The debug output will tell us exactly where
   - We can then fix that specific component

---

**Status:** ✅ Build complete and ready for immediate testing
**Modified:** `wii/src/main.cpp` (all critical initialization steps now have flushed debug output)
**Last Built:** 2026-02-14 19:44:59 CET
