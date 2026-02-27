#pragma once
#include "constants.h"
#include <set>
#include <SDL2/SDL.h>

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

private:
    static void pollGamepads();

    static inline std::set<Button> keyboardHeld_;
    static inline std::set<Button> gamepadHeld_;
    static inline std::set<Button> prevGamepadHeld_;
    static inline std::set<Button> pressed_;
    static inline std::set<Button> released_;
    static inline bool enabled_ = true;
    static inline SDL_GameController* controller_ = nullptr;

    static constexpr float STICK_DEADZONE = 0.3f;
};
