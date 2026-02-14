#pragma once
#include "screen.h"

class Animation;
class BitmapFont;

class TitleScreen : public Screen {
public:
    TitleScreen();

    void onEntered() override;
    void onDraw(SDL_Renderer* renderer) override;
    void onLogic() override;

private:
    bool hasContinue_ = false;
    BitmapFont* font_;
    int frameCounter_ = 0;
    bool gameStart_ = false;
    Animation* hand_;
    int selected_ = 0;
    Animation* title_;
};
