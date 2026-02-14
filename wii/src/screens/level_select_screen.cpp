#include "screens/level_select_screen.h"
#include "resource.h"
#include "media/animation.h"
#include "media/font.h"
#include "media/sound.h"
#include "media/music.h"
#include "input.h"
#include "game_state.h"
#include "constants.h"
#include "entities/dialogue.h"
#include "screens/level_screen.h"
#include "screens/end_screen.h"
#include <cstdlib>
#include <cmath>

bool LevelSelectScreen::bossLevelCompleted = false;

LevelSelectScreen::LevelSelectScreen() {
    background_ = Resource::getAnimation("data/images/level_select.bmp", 1);
    completedIcon_ = Resource::getAnimation("data/images/completed_level.bmp", 1);
    font_ = Resource::getFont("data/images/font.bmp");
    icons_ = Resource::getAnimation("data/images/icons.bmp", 9);
    selectedBg_ = Resource::getAnimation("data/images/selected_level_background.bmp", 6);
    unselectedBg_ = Resource::getAnimation("data/images/unselected_level_background.bmp", 1);

    for (int i = 0; i < 200; i++) {
        StarParticle p;
        p.x = static_cast<float>(rand() % 320);
        p.y = static_cast<float>(rand() % 240);
        p.z = static_cast<float>(rand() % 3000) / 1000.0f + 1.0f;
        particles_.push_back(p);
    }

    setLevel(0, 0, LevelDescription("GRAPPLIN' HEAVEN", "data/rooms/olof2.txt", 0, "data/music/olof9.xm"));
    setLevel(1, 0, LevelDescription("CAVERNOUS", "data/rooms/per4.txt", 1, "data/music/olof8.xm"));
    setLevel(2, 0, LevelDescription("HARRY U.P.", "data/rooms/level1.txt", 7, "data/music/olof2-rmx.xm"));
    setLevel(0, 1, LevelDescription("GLIDER", "data/rooms/per1.txt", 3, "data/music/olof12.xm"));
    setLevel(1, 1, LevelDescription("FINAL CORE", "data/rooms/olof.txt", 5, "data/music/spooky.xm"));
    setLevel(2, 1, LevelDescription("UNSTABLE", "data/rooms/breaktilelevel.txt", 4, "data/music/olof8.xm"));
    setLevel(0, 2, LevelDescription("THE TOWER", "data/rooms/per2.txt", 2, "data/music/olof12.xm"));
    setLevel(1, 2, LevelDescription("WALL OF DEATH", "data/rooms/per3.txt", 6, "data/music/olof2-rmx.xm"));
    setLevel(2, 2, LevelDescription("LAVA LAND", "data/rooms/levellava.txt", 8, "data/music/olof2-rmx.xm"));

    firstDialogue_ = new Dialogue("data/dialogues/level_select.txt");
    firstDialogue_->setRunWithoutHero();
    bossUnlockedDialogue_ = new Dialogue("data/dialogues/boss_unlocked.txt");
    bossUnlockedDialogue_->setRunWithoutHero();
}

LevelSelectScreen::~LevelSelectScreen() {
    delete firstDialogue_;
    delete bossUnlockedDialogue_;
}

void LevelSelectScreen::setLevel(int x, int y, const LevelDescription& desc) {
    levelDescs_[y * 3 + x] = desc;
}

void LevelSelectScreen::onEntered() {
    GameState::saveToFile();

    bossLevelUnlocked_ = true;
    for (int i = 0; i < 9; i++) {
        if (i == 4) continue;
        if (GameState::getInt(levelDescs_[i].levelFile) == 0) {
            bossLevelUnlocked_ = false;
            break;
        }
    }

    const LevelDescription& bossLevel = levelDescs_[4];
    if (GameState::getInt(bossLevel.levelFile) == 1 && playingBossLevel_ && bossLevelCompleted) {
        bossLevelCompleted = false;
        playingBossLevel_ = false;
        ScreenManager::add(new EndScreen());
        return;
    }

    levelSelected_ = false;
    levelSelectedCounter_ = 0;
    Music::playSong("data/music/level_select.xm");

    if (GameState::getInt("level_select_dialogue") == 0) {
        runFirstDialogue_ = true;
        GameState::put("level_select_dialogue", 1);
    }

    if (bossLevelUnlocked_ && GameState::getInt("level_boss_unlocked_dialogue") == 0) {
        runBossUnlockedDialogue_ = true;
        GameState::put("level_boss_unlocked_dialogue", 1);
    }
}

void LevelSelectScreen::onExited() {
    GameState::saveToFile();
}

void LevelSelectScreen::onDraw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 57, 56, 41, 255);
    SDL_Rect bg = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &bg);

    for (const auto& p : particles_) {
        if (p.z <= 2.0f)
            SDL_SetRenderDrawColor(renderer, 123, 113, 99, 255);
        else
            SDL_SetRenderDrawColor(renderer, 181, 166, 107, 255);
        SDL_Rect pr = {static_cast<int>(std::round(p.x)), static_cast<int>(std::round(p.y)), 2, 1};
        SDL_RenderFillRect(renderer, &pr);
    }

    if (!levelSelected_) background_->drawFrame(renderer, 0, 0, 0);

    if (levelSelected_ && levelSelectedCounter_ % 6 < 3 && levelSelectedCounter_ < 20) {
        SDL_SetRenderDrawColor(renderer, 231, 215, 156, 255);
        SDL_Rect flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &flash);
    }

    if (levelSelected_ && levelSelectedCounter_ < 10) return;

    if (!levelSelected_) {
        font_->drawCenter(renderer, "SELECT LEVEL", 0, 30, 320, 20);
    }

    int selIdx = selectedY_ * 3 + selectedX_;

    for (int i = 0; i < 9; i++) {
        int x = i % 3;
        int y = i / 3;
        const LevelDescription& desc = levelDescs_[i];

        Animation* anim = (selectedX_ == x && selectedY_ == y) ? selectedBg_ : unselectedBg_;
        int xPos = (x - 1) * (anim->getFrameWidth() + 5) - anim->getFrameWidth() / 2;
        int yPos = (y - 1) * (anim->getFrameHeight() + 5) - anim->getFrameHeight() / 2;

        anim->drawFrame(renderer, frameCounter_ / 3, 160 + xPos, 120 + yPos);

        if (!levelSelected_ || selIdx == i) {
            if ((i == 4 && bossLevelUnlocked_) || i != 4)
                icons_->drawFrame(renderer, desc.frameIndex, 164 + xPos, 124 + yPos);
        }
        if (!levelSelected_ && GameState::getInt(desc.levelFile) == 1) {
            completedIcon_->drawFrame(renderer, 0, 164 + xPos, 124 + yPos);
        }
    }

    if (!levelSelected_) {
        const LevelDescription& desc = levelDescs_[selIdx];
        font_->draw(renderer, desc.name, 220, 68);

        if (GameState::getInt(desc.levelFile) == 1) {
            font_->draw(renderer, "STATUS:", 220, 88);
            font_->draw(renderer, "   CLEARED", 220, 98);
        } else if (selIdx == 4 && !bossLevelUnlocked_) {
            font_->draw(renderer, "STATUS:", 220, 88);
            font_->draw(renderer, "   LOCKED", 220, 98);
        } else {
            font_->draw(renderer, "STATUS:", 220, 88);
            font_->draw(renderer, "   RADIOACTIVE", 220, 98);
        }
    }

    if (runFirstDialogue_) firstDialogue_->draw(renderer, 0, 0, 0);
    if (runBossUnlockedDialogue_) bossUnlockedDialogue_->draw(renderer, 0, 0, 0);
}

void LevelSelectScreen::onLogic() {
    frameCounter_++;

    for (auto& p : particles_) {
        p.x -= p.z / 2.0f;
        if (p.x < 0) {
            p.x = 320.0f;
            p.y = static_cast<float>(rand() % 240);
            p.z = static_cast<float>(rand() % 3000) / 1000.0f + 1.0f;
        }
    }

    if (levelSelected_) {
        levelSelectedCounter_++;
        if (levelSelectedCounter_ > 100) {
            int index = selectedY_ * 3 + selectedX_;
            playingBossLevel_ = (index == 4);
            ScreenManager::add(new LevelScreen(levelDescs_[index]));
        }
        return;
    }

    if (Input::isPressed(Button::EXIT)) {
        exit();
        Sound::playSample("data/sounds/select");
    }

    if (Input::isPressed(Button::LEFT)) {
        selectedX_--;
        if (selectedX_ < 0) selectedX_ = 0;
        else Sound::playSample("data/sounds/select");
    }
    if (Input::isPressed(Button::RIGHT)) {
        selectedX_++;
        if (selectedX_ > 2) selectedX_ = 2;
        else Sound::playSample("data/sounds/select");
    }
    if (Input::isPressed(Button::UP)) {
        selectedY_--;
        if (selectedY_ < 0) selectedY_ = 0;
        else Sound::playSample("data/sounds/select");
    }
    if (Input::isPressed(Button::DOWN)) {
        selectedY_++;
        if (selectedY_ > 2) selectedY_ = 2;
        else Sound::playSample("data/sounds/select");
    }

    if (Input::isPressed(Button::FIRE)) {
        int index = selectedY_ * 3 + selectedX_;
        if ((index == 4 && bossLevelUnlocked_) || index != 4) {
            Music::stop();
            Sound::playSample("data/sounds/start");
            levelSelected_ = true;
            levelSelectedCounter_ = 0;
        }
    }

    if (runFirstDialogue_) firstDialogue_->update();
    if (runBossUnlockedDialogue_) bossUnlockedDialogue_->update();
}
