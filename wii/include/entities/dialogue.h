#pragma once
#include "entity.h"
#include <string>
#include <vector>

class Animation;
class BitmapFont;

class Dialogue : public Entity {
public:
    Dialogue(const std::string& filename);

    int getLayer() const override { return 3; }

    void setRunWithoutHero();
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    enum class CharacterPortrait { DoctorGreen, Ted };

    struct DialogueLine {
        std::string text;
        CharacterPortrait portrait;
    };

    Animation* background_;
    int currentCharacter_ = 0;
    int currentLine_ = 0;
    Animation* doctorGreenPortrait_;
    bool done_ = false;
    std::string file_;
    BitmapFont* font_;
    int dialogueFrameCounter_ = 0;
    std::vector<DialogueLine> lines_;
    bool running_ = false;
    bool runWithoutHero_ = false;
    Animation* tedPortrait_;
};
