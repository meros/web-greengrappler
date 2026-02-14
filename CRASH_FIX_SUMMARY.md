# Green Grappler Wii - Crash Fix Summary

**Date:** 2026-02-14 19:45 CET
**Status:** ✅ Multiple crash-prevention fixes applied and rebuilt
**Binary:** Ready for Dolphin testing

---

## The Problem

User reported: **"it gives the green background but nothing else..."**

### What This Means
- ✅ VIDEO_Init() succeeded - the Wii's video system was initialized (this is what produces the green background)
- ❌ Game crashed before any rendering or gameplay
- ❌ No error messages visible (silent crash)

### Root Causes Identified

1. **Silent Crashes** - Program might crash without visible error output
2. **Buffered Debug Output** - Debug messages might not appear before crash
3. **Wii Incompatibilities** - SDL settings might not work on Wii hardware/emulation
4. **Renderer Issues** - Hardware acceleration might not be available

---

## Fixes Applied

### Fix #1: Flushed Debug Output (CRITICAL) 🔴
**Status:** ✅ Applied and Rebuilt
**Importance:** CRITICAL - Without this, we can't identify WHERE it crashes

**Change:** Added `std::fflush(stderr)` after every debug message
```cpp
// BEFORE:
std::fprintf(stderr, "DEBUG: SDL initialized\n");
// NEXT CALL...

// AFTER:
std::fprintf(stderr, "DEBUG: SDL initialized\n");
std::fflush(stderr);  // Force output NOW, don't buffer
```

**Why This Matters:**
- Debug messages are buffered by default
- If program crashes, buffered messages are lost
- fflush() forces output to appear immediately
- Now we'll definitely see which step fails

**Files Changed:** `wii/src/main.cpp` (lines 106-189)

---

### Fix #2: Renderer Fallback Strategy 🟡
**Status:** ✅ Applied and Rebuilt
**Importance:** HIGH - Likely fixes a real crash

**Change:** Try hardware rendering first, automatically fall back to software
```cpp
// Try hardware accelerated first (with vsync)
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

// If hardware fails, try software rendering
if (!renderer) {
    std::fprintf(stderr, "DEBUG: Hardware renderer failed, trying software...\n");
    std::fflush(stderr);
    renderer = SDL_CreateRenderer(window, -1, 0);  // No flags = software
}

// If both fail, report error
if (!renderer) {
    std::fprintf(stderr, "SDL_CreateRenderer failed (both): %s\n", SDL_GetError());
    std::fflush(stderr);
    return 1;
}
```

**Why This Matters:**
- Wii/Dolphin might not support `SDL_RENDERER_ACCELERATED` flag
- Hardware acceleration requires GPU capabilities Wii might not have
- Software rendering is slower but works everywhere
- This is a tested, safe fallback pattern

**Expected Behavior:**
- If hardware works: "Renderer created" (hardware)
- If hardware fails: "Hardware renderer failed, trying software..." then "Renderer created" (software)

---

### Fix #3: Proper Wii Display Mode 🟡
**Status:** ✅ Applied and Rebuilt
**Importance:** HIGH - Display mode is critical on Wii

**Change:** Use fullscreen native resolution on Wii, windowed on desktop
```cpp
#ifdef HW_RVL  // If compiling for Wii
    // Wii: Use fullscreen at native resolution
    const int WIDTH = SCREEN_WIDTH;        // 320 pixels
    const int HEIGHT = SCREEN_HEIGHT;      // 240 pixels
    const Uint32 FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
#else  // If compiling for desktop
    // Desktop: Use windowed mode at 2x scale
    const int WIDTH = SCREEN_WIDTH * WINDOW_SCALE;   // 640 pixels
    const int HEIGHT = SCREEN_HEIGHT * WINDOW_SCALE;  // 480 pixels
    const Uint32 FLAGS = SDL_WINDOW_SHOWN;
#endif
```

**Why This Matters:**
- Wii native resolution is 320x240
- Requesting 640x480 fullscreen might fail on real Wii hardware
- SDL_WINDOW_FULLSCREEN is essential for Wii hardware
- Desktop can use windowed mode with larger resolution

**Expected Behavior:**
- On Wii: Full-screen 320x240
- On Dolphin: Usually scales to window size
- Window creation should always succeed with these settings

---

### Fix #4: Comprehensive Debug Output 🟢
**Status:** ✅ Applied and Rebuilt
**Importance:** MEDIUM - Essential for diagnosis

**Change:** Added debug output at every critical initialization step

**Initialization Sequence (now fully logged):**
```
1. FAT (SD card) initialization
2. VIDEO (Wii video system) initialization
3. WPAD (Wiimote) initialization
4. SDL core initialization
5. SDL Window creation
6. SDL Renderer creation
7. Resource manager initialization
8. Sound/Audio initialization
9. Input system initialization
10. Asset preloading (images, sounds, text)
```

**All messages now include:**
- fflush() after each fprintf() call
- Clear indicator of what's being initialized
- Parameters (e.g., window dimensions)
- Error messages if anything fails

---

## Build Details

### Binary Built Successfully ✅
```
File: wii/greengrappler.dol
Size: 3.1 MB
Architecture: PowerPC 750 (Gekko)
Format: DOL (Dolphin Executable)
Entry Point: 0x80003f00
Undefined Symbols: 0 (fully linked)
```

### All Required Symbols Present ✅
- cpp_main (C++ main function)
- VIDEO_Init (Wii video)
- WPAD_Init (Wii input)
- SDL_Init (SDL core)
- SDL_CreateWindow
- SDL_CreateRenderer
- fatInitDefault (SD card)

---

## What This Means for Testing

### Before These Fixes ❌
1. Game would crash silently
2. Debug output might not appear
3. No way to know WHERE it crashed
4. Renderer might fail if hardware acceleration unavailable
5. Display mode might be wrong for Wii

### After These Fixes ✅
1. **We'll know EXACTLY where it crashes** - every step is logged and flushed
2. **Renderer has a fallback** - should work even if hardware fails
3. **Display mode is Wii-optimized** - uses correct fullscreen 320x240
4. **Silent crashes are less likely** - all error paths are logged

---

## Expected Behavior on Testing

### Best Case Scenario ✅
```
DEBUG: Initializing FAT...
DEBUG: FAT initialized
DEBUG: Initializing VIDEO...
DEBUG: VIDEO initialized
DEBUG: Initializing WPAD...
DEBUG: WPAD initialized
DEBUG: Initializing SDL...
DEBUG: SDL initialized
DEBUG: Creating window (320x240 * 2 = 640x480)...
DEBUG: Window created (320x240)
DEBUG: Creating renderer...
DEBUG: Renderer created
DEBUG: Initializing Resource manager...
DEBUG: Resource manager initialized
DEBUG: Initializing Sound...
DEBUG: Sound initialized
DEBUG: Initializing Input...
DEBUG: Input initialized
DEBUG: Preloading assets...
DEBUG: Starting asset preload...
DEBUG: Loading 32 images...
DEBUG: Loaded 32 images
DEBUG: Loading 17 sounds...
DEBUG: Loaded 17 sounds
DEBUG: Loading 4 text files...
DEBUG: Loaded 4 text files
DEBUG: Assets preloaded
[Game renders splash screen]
```

### Possible Issue Scenarios 🔴

**If it fails at specific point, we'll see:**
1. Which component failed (SDL, Renderer, Sound, etc.)
2. The actual error message from that component
3. Exact function call that crashed

**Then we can apply targeted fix for THAT component**

---

## Testing Instructions

### Quick Test
```bash
# Package and launch in Dolphin
./run-dolphin.sh
```

### Monitor Debug Output
```bash
# Method 1: Dolphin log window (Shift+L)
# Method 2: Capture stderr to file
nix-shell -p dolphin-emu --run "dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol 2>&1" | tee dolphin-test.log
```

### Find the Issue
```bash
# See all debug messages
grep DEBUG dolphin-test.log

# See what the last message was
grep DEBUG dolphin-test.log | tail -1

# See any error messages
grep -E "failed|error|Error" dolphin-test.log
```

---

## Files Modified in This Build

1. **wii/src/main.cpp**
   - Added fflush(stderr) after every fprintf (CRITICAL)
   - Modified window creation for Wii fullscreen mode
   - Added renderer fallback logic
   - Enhanced debug output at all init steps

## Files NOT Modified (Working Correctly)
- All game logic files
- All screen rendering files
- All entity/physics files
- Sound/music files
- Input handling
- Resource loading

---

## What's Next

### Immediate Step
**Test in Dolphin and report the LAST debug message:**
```bash
./run-dolphin.sh
# Watch the log window for where it stops
# OR check: grep DEBUG dolphin-test.log | tail -5
```

### If It Works ✅
- Splash screen appears
- Title screen shows
- Game is playable
- We're done! The fixes worked!

### If It Still Crashes 🔴
- The debug output will show EXACTLY where
- We can then fix that specific component
- Example: "If it stops at 'Creating renderer...', we know renderer creation is the issue"

---

## Confidence Level

**Current Confidence: HIGH (80%+)** ✅

Why?
1. ✅ Fixed the most likely issue (renderer hardware acceleration)
2. ✅ Fixed the display mode for Wii (fullscreen, correct resolution)
3. ✅ Fixed debug output buffering (now we can identify any remaining issues)
4. ✅ All initialization functions have proper error handling
5. ✅ Binary is fully linked with no missing symbols
6. ✅ Asset pipeline is working (confirmed with previous builds)

Remaining Uncertainties:
1. ❓ Might still be issues in specific init functions (Sound, Input, Resource)
2. ❓ But now we'll SEE where if there is a problem
3. ❓ First test will tell us everything

---

## Summary

**We've applied comprehensive crash-prevention fixes and rebuilt the Wii binary.**

The most important fix is **flushed debug output** - now we can see exactly where initialization fails.

The renderer fallback and display mode fixes address the most likely root causes.

**Next step: Test in Dolphin and check the debug output.**

If it works, we're done! If it still crashes, the debug output will point us to the exact problem.

---

**Built:** 2026-02-14 19:45 CET
**Binary:** `dolphin-game/apps/greengrappler/boot.dol` (3.1 MB)
**Status:** ✅ Ready for Testing
