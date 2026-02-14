#include "input.h"

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

void Input::init() {
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller_ = SDL_GameControllerOpen(i);
            break;
        }
    }
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
    if (event.type == SDL_CONTROLLERDEVICEADDED && !controller_) {
        controller_ = SDL_GameControllerOpen(event.cdevice.which);
    }
    if (event.type == SDL_CONTROLLERDEVICEREMOVED && controller_) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
}

void Input::pollGamepads() {
    gamepadHeld_.clear();

    if (!controller_) return;

    // D-pad
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_UP)) gamepadHeld_.insert(Button::UP);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) gamepadHeld_.insert(Button::DOWN);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) gamepadHeld_.insert(Button::LEFT);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) gamepadHeld_.insert(Button::RIGHT);

    // Buttons
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_A)) gamepadHeld_.insert(Button::JUMP);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_X)) gamepadHeld_.insert(Button::JUMP);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_B)) gamepadHeld_.insert(Button::FIRE);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_Y)) gamepadHeld_.insert(Button::FIRE);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_BACK)) gamepadHeld_.insert(Button::FORCE_QUIT);
    if (SDL_GameControllerGetButton(controller_, SDL_CONTROLLER_BUTTON_START)) gamepadHeld_.insert(Button::EXIT);

    // Left stick
    float lx = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
    float ly = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
    if (lx < -STICK_DEADZONE) gamepadHeld_.insert(Button::LEFT);
    if (lx > STICK_DEADZONE) gamepadHeld_.insert(Button::RIGHT);
    if (ly < -STICK_DEADZONE) gamepadHeld_.insert(Button::UP);
    if (ly > STICK_DEADZONE) gamepadHeld_.insert(Button::DOWN);

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
