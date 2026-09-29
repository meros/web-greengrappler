#include "screens/title_screen.h"
#include "resource.h"
#include "media/animation.h"
#include "media/font.h"
#include "media/sound.h"
#include "media/music.h"
#include "input.h"
#include "game_state.h"
#include "constants.h"
#include "level_description.h"
#include "screens/level_select_screen.h"
#include "screens/level_screen.h"
#include <cstdio>

TitleScreen::TitleScreen() {
    std::fprintf(stderr, "DEBUG: TitleScreen constructor\n"); std::fflush(stderr);
    font_ = Resource::getFont("data/images/font.bmp");
    hand_ = Resource::getAnimation("data/images/hand.bmp", 1);
    title_ = Resource::getAnimation("data/images/title.bmp", 1);
    std::fprintf(stderr, "DEBUG: TitleScreen constructor done\n"); std::fflush(stderr);
}

void TitleScreen::onEntered() {
    gameStart_ = false;
    Music::playSong("data/music/intro2.xm");
    hasContinue_ = GameState::isSavePresent();
    if (hasContinue_) selected_ = 1;
}

void TitleScreen::onDraw(SDL_Renderer* renderer) {
    title_->drawFrame(renderer, 0, 0, 0);

    // Draw controls help text
#ifdef HW_RVL
    font_->drawCenter(renderer, "D-PAD MOVE  2-JUMP  1-ROPE", 0, 115, 320, 10);
#else
    font_->drawCenter(renderer, "ARROWS-MOVE SPACE-JUMP CTRL-ROPE", 0, 115, 320, 10);
#endif

    if (!gameStart_) {
        hand_->drawFrame(renderer, 0, 115, 150 + selected_ * 10);
#ifdef HW_RVL
        if (Input::pointerValid())
            hand_->drawFrame(renderer, 0, Input::pointerX(), Input::pointerY());
#endif
    }

    if (!gameStart_ || (selected_ != 0 || frameCounter_ % 10 < 5))
        font_->draw(renderer, "NEW GAME", 126, 150);

    if (hasContinue_) {
        if (!gameStart_ || (selected_ != 1 || frameCounter_ % 10 < 5))
            font_->draw(renderer, "CONTINUE", 126, 160);
        font_->draw(renderer, "EXIT GAME", 126, 170);
    } else {
        font_->draw(renderer, "EXIT GAME", 126, 160);
    }

    font_->draw(renderer, "DARKBITS", 130, 215);
    font_->draw(renderer, "SPEEDHACK 2011", 113, 225);
}

void TitleScreen::onLogic() {
    frameCounter_++;

    if (gameStart_) {
        if (frameCounter_ > 100) {
            ScreenManager::add(new LevelSelectScreen());
            if (selected_ == 0) {
                ScreenManager::add(new LevelScreen(LevelDescription(
                    "tutorial", "data/rooms/tutorial.txt", 0, "data/music/olof9.xm")));
            }
        }
        return;
    }

#ifdef HW_RVL
    // IR pointer hit-testing on menu items
    if (Input::pointerValid()) {
        int px = Input::pointerX();
        int py = Input::pointerY();
        if (px >= 115 && px <= 210) {
            int maxItem = hasContinue_ ? 2 : 1;
            for (int i = 0; i <= maxItem; i++) {
                if (py >= 150 + i * 10 && py < 150 + i * 10 + 10) {
                    if (selected_ != i) {
                        selected_ = i;
                        Sound::playSample("data/sounds/select");
                    }
                    break;
                }
            }
        }
    }
#endif

    if (Input::isPressed(Button::DOWN)) {
        selected_++;
        if (selected_ > 1 && !hasContinue_) selected_ = 1;
        else if (selected_ > 2) selected_ = 2;
        else Sound::playSample("data/sounds/select");
    }

    if (Input::isPressed(Button::UP)) {
        selected_--;
        if (selected_ < 0) selected_ = 0;
        else Sound::playSample("data/sounds/select");
    }

    if (Input::isPressed(Button::EXIT)) exit();

    if (Input::isPressed(Button::FIRE) || Input::isPressed(Button::JUMP)) {
        if (selected_ == 0) {
            gameStart_ = true;
            frameCounter_ = 0;
            Music::stop();
            Sound::playSample("data/sounds/start");
            GameState::clear();
        }
        if (selected_ == 1 && hasContinue_) {
            gameStart_ = true;
            frameCounter_ = 0;
            Music::stop();
            Sound::playSample("data/sounds/start");
            GameState::loadFromFile();
        }
        if (selected_ == 1 && !hasContinue_) exit();
        if (selected_ == 2 && hasContinue_) exit();
    }
}
