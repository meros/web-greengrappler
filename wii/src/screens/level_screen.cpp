#include "screens/level_screen.h"
#include "resource.h"
#include "media/animation.h"
#include "media/font.h"
#include "media/sound.h"
#include "media/music.h"
#include "input.h"
#include "game_state.h"
#include "constants.h"
#include "room.h"
#include "room_loader.h"
#include "screens/level_select_screen.h"
#include <string>

LevelScreen::LevelScreen(const LevelDescription& desc)
    : levelDesc_(desc)
{
    darken_ = Resource::getAnimation("data/images/darken.bmp");
    exitBg_ = Resource::getAnimation("data/images/level_exit_background.bmp", 1);
    font_ = Resource::getFont("data/images/font.bmp");
    hand_ = Resource::getAnimation("data/images/hand.bmp", 1);

    Music::playSong(desc.musicFile);
    room_ = loadRoom(desc.levelFile);
}

LevelScreen::~LevelScreen() {
    delete room_;
}

void LevelScreen::onDraw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 57, 56, 41, 255);
    SDL_Rect bg = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &bg);

    room_->onDraw(renderer);

    int coins = GameState::getInt("coins");
    std::string coinsStr = "[x" + std::to_string(coins);

    SDL_SetRenderDrawColor(renderer, 57, 56, 41, 255);
    SDL_Rect topBar = {0, 0, SCREEN_WIDTH, 10};
    SDL_RenderFillRect(renderer, &topBar);
    font_->draw(renderer, coinsStr, 1, 1);

    if (isExit_) {
        for (int y = 0; y < 15; y++)
            for (int x = 0; x < 20; x++)
                darken_->drawFrame(renderer, 0, x * 16, y * 16);

        exitBg_->drawFrame(renderer, 0,
            160 - exitBg_->getFrameWidth() / 2,
            120 - exitBg_->getFrameHeight() / 2);
        font_->draw(renderer, "CONTINUE", 130, 110);
        font_->draw(renderer, "EXIT LEVEL", 130, 120);
        hand_->drawFrame(renderer, 0, 120, 110 + selected_ * 10);
    }
}

void LevelScreen::onLogic() {
    if (room_->isCompleted()) {
        if (levelDesc_.levelFile == "data/rooms/olof.txt")
            LevelSelectScreen::bossLevelCompleted = true;
        GameState::put(levelDesc_.levelFile, 1);
        exit();
    }

    if (!isExit_) room_->onLogic();

    if (Input::isPressed(Button::EXIT) && !isExit_) {
        Sound::playSample("data/sounds/select");
        isExit_ = true;
    } else if (Input::isPressed(Button::EXIT) && isExit_) {
        Sound::playSample("data/sounds/select");
        isExit_ = false;
    }

    if (!isExit_) return;

    if (Input::isPressed(Button::DOWN)) {
        selected_++;
        if (selected_ > 1) selected_ = 1;
        else Sound::playSample("data/sounds/select");
    }
    if (Input::isPressed(Button::UP)) {
        selected_--;
        if (selected_ < 0) selected_ = 0;
        else Sound::playSample("data/sounds/select");
    }
    if (Input::isPressed(Button::FIRE)) {
        Sound::playSample("data/sounds/select");
        if (selected_ == 0) isExit_ = false;
        else if (selected_ == 1) exit();
    }
}
