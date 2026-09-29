#pragma once
#include "constants.h"
#include <set>
#include <SDL2/SDL.h>
#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif

class Input {
public:
    static void init();
    static void handleEvent(const SDL_Event& event);
    static void update();
    static bool isHeld(Button button);
    static bool isPressed(Button button);
    static bool isReleased(Button button);
    static void enable();
    static void disable();
    static bool hasController() { return joystick_ != nullptr || controller_ != nullptr; }
#ifdef HW_RVL
    static int pointerX() { return pointerX_; }
    static int pointerY() { return pointerY_; }
    static bool pointerValid() { return pointerValid_; }
#endif

private:
    static void pollGamepads();
    static void tryOpenJoystick();

    static inline std::set<Button> keyboardHeld_;
    static inline std::set<Button> gamepadHeld_;
    static inline std::set<Button> prevGamepadHeld_;
    static inline std::set<Button> pressed_;
    static inline std::set<Button> released_;
    static inline bool enabled_ = true;
    static inline SDL_GameController* controller_ = nullptr;
    static inline SDL_Joystick* joystick_ = nullptr;
    static inline Uint8 hatState_ = SDL_HAT_CENTERED;
    static inline Uint32 lastJoystickRetry_ = 0;
    static inline bool hasNunchuk_ = false;
#ifdef HW_RVL
    static inline int pointerX_ = 0;
    static inline int pointerY_ = 0;
    static inline bool pointerValid_ = false;
#endif

    static constexpr float STICK_DEADZONE = 0.3f;
};
