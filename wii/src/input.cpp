#include "input.h"
#include <cstdio>
#include <cstring>

static Button sdlKeyToButton(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP: case SDLK_w: return Button::UP;
        case SDLK_DOWN: case SDLK_s: return Button::DOWN;
        case SDLK_LEFT: case SDLK_a: return Button::LEFT;
        case SDLK_RIGHT: case SDLK_d: return Button::RIGHT;
        case SDLK_SPACE: return Button::JUMP;
        case SDLK_LCTRL: case SDLK_RCTRL: case SDLK_RETURN: return Button::FIRE;
        case SDLK_ESCAPE: return Button::FORCE_QUIT;
        case SDLK_p: return Button::EXIT;
        default: return Button::COUNT;
    }
}

// Map SDL joystick button index to game button.
// SDL2 Wii backend maps: 0=A, 1=B, 2=1, 3=2, 4=-, 5=+, 6=Home, 7=Z, 8=C
// D-pad is mapped as hat (handled separately).
static Button joyButtonToButton(int index) {
    switch (index) {
        case 0: return Button::JUMP;       // A
        case 1: return Button::FIRE;       // B
        case 2: return Button::FIRE;       // 1 → grapple
        case 3: return Button::JUMP;       // 2 → jump
        case 5: return Button::EXIT;       // +
        case 6: return Button::FORCE_QUIT; // Home
        case 7: return Button::JUMP;       // Nunchuk Z
        case 8: return Button::FIRE;       // Nunchuk C
        default: return Button::COUNT;
    }
}

void Input::tryOpenJoystick() {
    int numJoy = SDL_NumJoysticks();
    for (int i = 0; i < numJoy; i++) {
        SDL_Joystick* joy = SDL_JoystickOpen(i);
        if (joy) {
            const char* name = SDL_JoystickName(joy);
            // Prefer Wiimote for analog polling, fall back to first device
            if (!joystick_ || (name && std::strstr(name, "Wiimote"))) {
                joystick_ = joy;
            }
        }
    }
}

void Input::init() {
#ifdef HW_RVL
    // On Wii, use joystick API (Wiimotes appear as joysticks, not game controllers)
    std::fprintf(stderr, "DEBUG: Input::init - %d joysticks found\n", SDL_NumJoysticks());
    std::fflush(stderr);
    tryOpenJoystick();
#else
    // On desktop, try game controller first, then joystick
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller_ = SDL_GameControllerOpen(i);
            break;
        }
    }
    if (!controller_) {
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            joystick_ = SDL_JoystickOpen(i);
            if (joystick_) break;
        }
    }
#endif
}

void Input::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        Button btn = sdlKeyToButton(event.key.keysym.sym);
        if (btn != Button::COUNT) {
            keyboardHeld_.insert(btn);
            pressed_.insert(btn);
        }
    }
    if (event.type == SDL_KEYUP) {
        Button btn = sdlKeyToButton(event.key.keysym.sym);
        if (btn != Button::COUNT) {
            keyboardHeld_.erase(btn);
            released_.insert(btn);
        }
    }
#ifdef HW_RVL
    // Open all joystick devices as they appear (GC + Wiimote)
    if (event.type == SDL_JOYDEVICEADDED) {
        SDL_Joystick* joy = SDL_JoystickOpen(event.jdevice.which);
        if (joy) {
            const char* name = SDL_JoystickName(joy);
            if (!joystick_ || (name && std::strstr(name, "Wiimote"))) {
                joystick_ = joy;
            }
        }
    }
#else
    if (event.type == SDL_CONTROLLERDEVICEADDED && !controller_) {
        controller_ = SDL_GameControllerOpen(event.cdevice.which);
    }
    if (event.type == SDL_CONTROLLERDEVICEREMOVED && controller_) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
#endif
    // Track hat state — directions are reconstructed each frame in pollGamepads()
    if (event.type == SDL_JOYHATMOTION && joystick_) {
        hatState_ = event.jhat.value;
    }
    // Handle joystick button events
    if (event.type == SDL_JOYBUTTONDOWN && joystick_) {
        Button btn = joyButtonToButton(event.jbutton.button);
        if (btn != Button::COUNT) {
            pressed_.insert(btn);
            gamepadHeld_.insert(btn);
        }
    }
    if (event.type == SDL_JOYBUTTONUP && joystick_) {
        Button btn = joyButtonToButton(event.jbutton.button);
        if (btn != Button::COUNT) {
            released_.insert(btn);
            gamepadHeld_.erase(btn);
        }
    }
}

void Input::pollGamepads() {
    // Poll game controller (desktop)
    if (controller_) {
        gamepadHeld_.clear();
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_UP)) gamepadHeld_.insert(Button::UP);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) gamepadHeld_.insert(Button::DOWN);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) gamepadHeld_.insert(Button::LEFT);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) gamepadHeld_.insert(Button::RIGHT);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_A)) gamepadHeld_.insert(Button::JUMP);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_X)) gamepadHeld_.insert(Button::JUMP);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_B)) gamepadHeld_.insert(Button::FIRE);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_Y)) gamepadHeld_.insert(Button::FIRE);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_BACK)) gamepadHeld_.insert(Button::FORCE_QUIT);
        if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_START)) gamepadHeld_.insert(Button::EXIT);

        float lx = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
        float ly = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
        if (lx < -STICK_DEADZONE) gamepadHeld_.insert(Button::LEFT);
        if (lx > STICK_DEADZONE) gamepadHeld_.insert(Button::RIGHT);
        if (ly < -STICK_DEADZONE) gamepadHeld_.insert(Button::UP);
        if (ly > STICK_DEADZONE) gamepadHeld_.insert(Button::DOWN);
    }

    // Keep scanning for new joystick devices — Wiimotes connect async via
    // Bluetooth and may appear after the GC controller is already opened.
    // SDL_JoystickOpen on an already-opened device is a safe no-op.
    {
        Uint32 now = SDL_GetTicks();
        if (now - lastJoystickRetry_ >= 500) {
            lastJoystickRetry_ = now;
            tryOpenJoystick();
        }
    }

    // Rebuild directions each frame from hat + analog stick
    if (joystick_ && !controller_) {
        gamepadHeld_.erase(Button::UP);
        gamepadHeld_.erase(Button::DOWN);
        gamepadHeld_.erase(Button::LEFT);
        gamepadHeld_.erase(Button::RIGHT);

        // Detect nunchuk by checking if analog axes are present
        int numAxes = SDL_JoystickNumAxes(joystick_);
        hasNunchuk_ = (numAxes >= 2);

        // D-pad hat — rotate for sideways Wiimote when no nunchuk
        if (hasNunchuk_) {
            // Normal orientation (nunchuk/classic controller)
            if (hatState_ & SDL_HAT_UP) gamepadHeld_.insert(Button::UP);
            if (hatState_ & SDL_HAT_DOWN) gamepadHeld_.insert(Button::DOWN);
            if (hatState_ & SDL_HAT_LEFT) gamepadHeld_.insert(Button::LEFT);
            if (hatState_ & SDL_HAT_RIGHT) gamepadHeld_.insert(Button::RIGHT);
        } else {
            // Sideways Wiimote: D-pad rotated 90° clockwise
            if (hatState_ & SDL_HAT_UP) gamepadHeld_.insert(Button::LEFT);
            if (hatState_ & SDL_HAT_DOWN) gamepadHeld_.insert(Button::RIGHT);
            if (hatState_ & SDL_HAT_LEFT) gamepadHeld_.insert(Button::DOWN);
            if (hatState_ & SDL_HAT_RIGHT) gamepadHeld_.insert(Button::UP);
        }

        // Analog stick (nunchuk or classic controller)
        if (hasNunchuk_) {
            float lx = SDL_JoystickGetAxis(joystick_, 0) / 32768.0f;
            float ly = SDL_JoystickGetAxis(joystick_, 1) / 32768.0f;
            if (lx < -STICK_DEADZONE) gamepadHeld_.insert(Button::LEFT);
            if (lx > STICK_DEADZONE) gamepadHeld_.insert(Button::RIGHT);
            if (ly < -STICK_DEADZONE) gamepadHeld_.insert(Button::UP);
            if (ly > STICK_DEADZONE) gamepadHeld_.insert(Button::DOWN);
        }
    }

#ifdef HW_RVL
    // Read IR pointer data from WPAD
    WPAD_ScanPads();
    WPADData* wpad = WPAD_Data(0);
    if (wpad && wpad->ir.valid) {
        pointerX_ = (int)((wpad->ir.x - OVERSCAN_X) * SCREEN_WIDTH / (float)VIEWPORT_W);
        pointerY_ = (int)((wpad->ir.y - OVERSCAN_Y) * SCREEN_HEIGHT / (float)VIEWPORT_H);
        pointerValid_ = (pointerX_ >= 0 && pointerX_ < SCREEN_WIDTH &&
                         pointerY_ >= 0 && pointerY_ < SCREEN_HEIGHT);
    } else {
        pointerValid_ = false;
    }
#endif

    // Compute pressed/released from gamepad state changes
    for (auto btn : gamepadHeld_) {
        if (prevGamepadHeld_.find(btn) == prevGamepadHeld_.end()) {
            pressed_.insert(btn);
        }
    }
    for (auto btn : prevGamepadHeld_) {
        if (gamepadHeld_.find(btn) == gamepadHeld_.end()) {
            released_.insert(btn);
        }
    }
    prevGamepadHeld_ = gamepadHeld_;
}

void Input::update() {
    pressed_.clear();
    released_.clear();
    pollGamepads();
}

bool Input::isHeld(Button button) {
    if (!enabled_) return false;
    return keyboardHeld_.count(button) || gamepadHeld_.count(button);
}

bool Input::isPressed(Button button) {
    return enabled_ && pressed_.count(button);
}

bool Input::isReleased(Button button) {
    return enabled_ && released_.count(button);
}

void Input::enable() { enabled_ = true; }
void Input::disable() { enabled_ = false; }
