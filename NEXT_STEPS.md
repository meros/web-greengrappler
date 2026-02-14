# Green Grappler Wii Build - Next Steps

**Current Status:** ✅ Binary rebuilt with crash-prevention fixes
**Ready for:** Dolphin emulator testing
**Time:** Now (2026-02-14 19:46 CET)

---

## ⚡ IMMEDIATE ACTION (Do This Now)

### Option 1: Quick Test (Easiest)
```bash
./run-dolphin.sh
```

### Option 2: Direct Launch
```bash
dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol
```

### Option 3: Via Nix (If Dolphin not installed)
```bash
nix-shell -p dolphin-emu --run "dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol"
```

---

## 👀 WHAT TO WATCH FOR

### 1. Game Launch (First 2 seconds)
- ✅ Green background should appear (Wii video initialization)
- ✅ Window/emulator should show the game running
- ❌ If it crashes immediately, it's a hardware/initialization issue

### 2. Debug Output (Critical!)
**Press Shift+L in Dolphin to open the log window**

You should see messages like:
```
DEBUG: Initializing FAT...
DEBUG: FAT initialized
DEBUG: Initializing VIDEO...
...
```

- ✅ All messages appear → Good initialization
- ❌ Messages stop suddenly → That's the failure point
- ❌ No messages at all → Buffer flush issue (shouldn't happen now)

### 3. Splash Screen
After all debug messages:
- ✅ Splash screen should fade in
- ✅ Animation should play
- ✅ Should transition to title screen

### 4. Title Screen
- ✅ Title screen with "Green Grappler" text
- ✅ Menu options visible
- ✅ D-Pad should navigate menu
- ✅ A button should select

### 5. Gameplay
- ✅ Game starts with level 1
- ✅ Character is visible and responsive
- ✅ Controls work (D-Pad, A for jump, B for hook)

---

## 📋 DETAILED DEBUG CHECKLIST

### If You See These Messages - ALL GOOD ✅
```
✅ DEBUG: FAT initialized
✅ DEBUG: VIDEO initialized
✅ DEBUG: WPAD initialized
✅ DEBUG: SDL initialized
✅ DEBUG: Window created (320x240)
✅ DEBUG: Renderer created
✅ DEBUG: Resource manager initialized
✅ DEBUG: Sound initialized
✅ DEBUG: Input initialized
✅ DEBUG: Loaded 32 images
✅ DEBUG: Loaded 17 sounds
✅ DEBUG: Loaded 4 text files
✅ DEBUG: Assets preloaded
[GAME RENDERS]
```
→ **Everything is working. Game should be playable!**

### If You See This Message + Error - Problem Identified 🔴

**Example Error Messages and Their Meanings:**

```
Last: DEBUG: Initializing SDL...
      SDL_Init failed: No available video driver
→ SDL can't find graphics driver on this system
  (Unlikely on Dolphin, but would indicate SDL issue)

Last: DEBUG: Creating renderer...
      Hardware renderer failed, trying software...
      DEBUG: Renderer created
→ NORMAL! Hardware acceleration unavailable, using software rendering
  (This is expected and should work fine)

Last: DEBUG: Loading 17 sounds...
      Loaded 15 sounds
      (game crashes or hangs here)
→ Specific sound file is missing or corrupted
  (Check that sound files exist in data/sounds/)

Last: DEBUG: Initializing Resource manager...
→ IMG_Init() (image library) failed
  (Check SDL_image library is working)
```

---

## 🔧 IF IT DOESN'T WORK

### Step 1: Capture the Debug Output
```bash
# Run and save all output to file
nix-shell -p dolphin-emu --run "dolphin-emu -e dolphin-game/apps/greengrappler/boot.dol 2>&1" | tee dolphin-output.log

# See the debug messages
grep DEBUG dolphin-output.log

# See the last few lines (error might be there)
tail -20 dolphin-output.log

# See everything
cat dolphin-output.log
```

### Step 2: Note the Last Message
```bash
grep DEBUG dolphin-output.log | tail -3
```
Copy/save this information.

### Step 3: Look for Error Messages
```bash
# Search for any error-related text
grep -i "error\|failed\|cannot\|no such" dolphin-output.log

# Or just see the last 30 lines
tail -30 dolphin-output.log
```

### Step 4: Report Back
When asking for help, provide:
1. **The last DEBUG message** (tells us where it fails)
2. **Any error messages** (tells us why)
3. **Your Dolphin version** (sometimes matters)
4. **Your system** (Windows/Mac/Linux)

---

## 📊 EXPECTED OUTCOMES

### Best Case ✅✅✅
```
Result: Game runs fully
Time: 30 seconds
Signs:
  - All debug messages appear
  - Splash screen shows
  - Title screen displays
  - Game starts when you press A
  - Controls respond to input
  - Graphics render correctly
Action: No further fixes needed! Ship it! 🚀
```

### Good Case ✅✅
```
Result: Game gets further than before
Time: 5-10 seconds
Signs:
  - More debug messages appear than before
  - Gets past initialization steps that failed before
  - New error message visible that wasn't before
Action: Now we know what to fix next
```

### Needs Debugging ✅
```
Result: Game crashes at specific point
Time: Immediate or during initialization
Signs:
  - Debug messages stop at specific line
  - Clear error message visible
  - We can see exactly which component fails
Action: Fix that component and rebuild
```

### No Visible Output ❌
```
Result: Green screen, no debug output
Time: Immediate
Signs:
  - Green background appears
  - No debug messages in log
  - Game crashes silently
Action: Unlikely now (we added buffer flushing)
  But if it happens: System might not be running our new binary
  Solution: Rebuild and repackage
```

---

## 🎮 CONTROLS REFERENCE

Once game is running, these should work:

```
D-Pad UP/DOWN/LEFT/RIGHT  - Move character
A Button                  - Jump
B Button                  - Fire grappling hook
HOME Button               - Quit to menu
```

On desktop keyboard (if running on PC Dolphin):
```
W/A/S/D or Arrow Keys   - Move
SPACE                   - Jump
CTRL or RETURN          - Grapple
ESC                     - Quit
```

---

## 📈 PROGRESS TRACKING

Use this to track what's working:

| Test | Before | After | Status |
|------|--------|-------|--------|
| Launches | ❌ Crash | ?      | TBD |
| Debug Output | ❌ Silent | ? | TBD |
| Renderer | ❌ ? | ? | TBD |
| Window | ❌ ? | ? | TBD |
| Splash | ❌ Never reached | ? | TBD |
| Title | ❌ Never reached | ? | TBD |
| Game Start | ❌ Never reached | ? | TBD |
| Gameplay | ❌ Never reached | ? | TBD |

After testing, report back what you see and we'll update this!

---

## 🚨 TROUBLESHOOTING QUICK LINKS

See these docs for detailed help:

1. **TESTING_GUIDE.md**
   - Detailed testing instructions
   - How to view debug output multiple ways
   - Troubleshooting by specific error message

2. **CRASH_FIX_SUMMARY.md**
   - What we fixed and why
   - Before/after analysis
   - Technical details of each fix

3. **DEBUG_SESSION_COMPLETE.md**
   - Full session summary
   - Complete documentation
   - What's ready to test

4. **BUILD_COMPLETE.md**
   - Build verification details
   - Binary information
   - Asset status

---

## ✅ FINAL CHECKLIST

Before testing, verify you have:

- [ ] Binary exists: `dolphin-game/apps/greengrappler/boot.dol` (3.1 MB)
- [ ] Assets exist: `dolphin-game/apps/greengrappler/data/` directory
- [ ] Dolphin installed: `which dolphin-emu` or `nix-shell -p dolphin-emu`
- [ ] Scripts ready: `./run-dolphin.sh` exists and is executable
- [ ] Documentation: You've read TESTING_GUIDE.md

If all checked:
```bash
./run-dolphin.sh
```

---

## 🎯 SUCCESS CRITERIA

Game is working if:
1. ✅ Launches without crashing
2. ✅ Shows splash screen after debug messages
3. ✅ Title screen displays menu
4. ✅ Game starts when you press A
5. ✅ Character is visible on screen
6. ✅ Controls respond (D-Pad moves, A jumps, B grapples)
7. ✅ Can complete at least one level

Current status: **Ready to test #1-3 above**

---

## 📞 IF YOU NEED HELP

After testing, provide:
1. **What happened** - Describe what you saw/didn't see
2. **Last DEBUG message** - The final "DEBUG:" line from the log
3. **Error messages** - Copy any error text after the last DEBUG
4. **Time until crash** - Did it crash immediately or after splash screen?
5. **Dolphin version** - What version of Dolphin are you using?

With this info, I can identify the exact issue and apply a targeted fix.

---

## 🚀 NEXT PHASE

**If it works:** Game is ready!
- Can be deployed to Wii via Homebrew Channel
- Can be tested on real hardware
- Ready for release

**If it doesn't work:** We have a clear path forward
- Debug output will show us exactly what fails
- We can fix that specific component
- Each test iteration gets us closer to success

---

**Status:** ✅ Ready to Test
**Binary:** Fully built and verified
**Documentation:** Complete
**Your turn:** Run the test! 🎮

```bash
./run-dolphin.sh
```

Good luck! 🍀

---

*Session: 2026-02-14 19:46 CET*
*Fixes: Flushed debug output, renderer fallback, Wii display mode*
*Next: Test in Dolphin and report results*
