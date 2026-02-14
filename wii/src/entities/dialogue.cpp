#include "entities/dialogue.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "media/font.h"
#include "room.h"
#include "game_state.h"
#include "constants.h"
#include <sstream>

Dialogue::Dialogue(const std::string& filename)
    : file_(filename) {
    background_ = Resource::getAnimation("data/images/dialogue.bmp", 1);
    doctorGreenPortrait_ = Resource::getAnimation("data/images/doctor_green_portrait.bmp", 1);
    tedPortrait_ = Resource::getAnimation("data/images/ted_portrait.bmp", 1);
    font_ = Resource::getFont("data/images/font.bmp");

    std::string text = Resource::getText(filename);
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        char ch = line[0];
        CharacterPortrait portrait = (ch == 'D') ? CharacterPortrait::DoctorGreen : CharacterPortrait::Ted;
        lines_.push_back({line.substr(1), portrait});
    }

    setSize(Vec2(10, 10 * 200));
}

void Dialogue::setRunWithoutHero() {
    runWithoutHero_ = true;
    running_ = true;
}

void Dialogue::draw(SDL_Renderer* renderer, int /*offsetX*/, int /*offsetY*/, int /*layer*/) {
    if (done_ || !running_) return;

    int position = SCREEN_HEIGHT - background_->getFrameHeight();

    background_->drawFrame(renderer, 0, 0, position);

    if (currentLine_ >= static_cast<int>(lines_.size())) return;
    const DialogueLine& line = lines_[currentLine_];

    int pX = 4;
    int pY = position + background_->getFrameHeight() - tedPortrait_->getFrameHeight() - 3;

    if (line.portrait == CharacterPortrait::Ted)
        tedPortrait_->drawFrame(renderer, 0, pX, pY);
    if (line.portrait == CharacterPortrait::DoctorGreen)
        doctorGreenPortrait_->drawFrame(renderer, 0, pX, pY);

    int fontPosition = position + background_->getFrameHeight() - 16;

    font_->drawWrap(renderer, line.text, 55, fontPosition, 300, currentCharacter_);
}

void Dialogue::update() {
    if (done_) return;

    if (!runWithoutHero_) {
        if (room_->getHero()->Collides(*this)) {
            running_ = true;
        }
    }

    if (!running_) return;

    dialogueFrameCounter_++;

    if (currentLine_ >= static_cast<int>(lines_.size())) return;
    const DialogueLine& line = lines_[currentLine_];

    if (dialogueFrameCounter_ > 2 && currentCharacter_ < static_cast<int>(line.text.length())) {
        currentCharacter_++;
        dialogueFrameCounter_ = 0;
        Sound::playSample("data/sounds/beep");
    }

    if (dialogueFrameCounter_ > 90 && static_cast<int>(lines_.size()) > currentLine_) {
        currentLine_++;
        currentCharacter_ = 0;
        dialogueFrameCounter_ = 0;

        if (static_cast<int>(lines_.size()) == currentLine_) {
            std::string key = "dialogue_" + file_;
            GameState::put(key, 1);
            done_ = true;
            if (!runWithoutHero_) remove();
        }
    }
}
