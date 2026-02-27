#pragma once
#include "screen.h"

class BitmapFont;

class EndScreen : public Screen {
public:
    EndScreen();

    void onDraw(SDL_Renderer* renderer) override;
    void onLogic() override;

private:
    static constexpr int GREEN_PEACE_DURATION = 60 * 7;

    int creditsOffset_ = 240;
    BitmapFont* font_;
    int frameCounter_ = -60;
};
