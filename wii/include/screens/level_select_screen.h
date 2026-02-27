#pragma once
#include "screen.h"
#include "level_description.h"
#include <vector>

class Animation;
class BitmapFont;
class Dialogue;

struct StarParticle {
    float x;
    float y;
    float z;
};

class LevelSelectScreen : public Screen {
public:
    static bool bossLevelCompleted;

    LevelSelectScreen();
    ~LevelSelectScreen();

    void onEntered() override;
    void onExited() override;
    void onDraw(SDL_Renderer* renderer) override;
    void onLogic() override;

private:
    void setLevel(int x, int y, const LevelDescription& desc);

    Animation* background_;
    bool bossLevelUnlocked_ = false;
    Dialogue* bossUnlockedDialogue_;
    Animation* completedIcon_;
    Dialogue* firstDialogue_;
    BitmapFont* font_;
    int frameCounter_ = 0;
    Animation* icons_;
    LevelDescription levelDescs_[9];
    bool levelSelected_ = false;
    int levelSelectedCounter_ = 0;
    std::vector<StarParticle> particles_;
    bool playingBossLevel_ = false;
    bool runBossUnlockedDialogue_ = false;
    bool runFirstDialogue_ = false;
    Animation* selectedBg_;
    int selectedX_ = 0;
    int selectedY_ = 0;
    Animation* unselectedBg_;
};
