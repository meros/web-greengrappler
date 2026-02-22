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
    static bool hasController() { return joystick_ != nullptr || controller_ != nullptr; }

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

    static constexpr float STICK_DEADZONE = 0.3f;
};
