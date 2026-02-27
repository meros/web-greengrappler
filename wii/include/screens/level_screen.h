#pragma once
#include "screen.h"
#include "level_description.h"

class Animation;
class BitmapFont;
class Room;

class LevelScreen : public Screen {
public:
    LevelScreen(const LevelDescription& desc);
    ~LevelScreen();

    void onDraw(SDL_Renderer* renderer) override;
    void onLogic() override;

private:
    Animation* darken_;
    bool isExit_ = false;
    Animation* exitBg_;
    BitmapFont* font_;
    Animation* hand_;
    LevelDescription levelDesc_;
    Room* room_;
    int selected_ = 0;
};
