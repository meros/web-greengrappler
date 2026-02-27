#pragma once
#include "screen.h"

class Animation;

class SplashScreen : public Screen {
public:
    SplashScreen();

    void onDraw(SDL_Renderer* renderer) override;
    void onLogic() override;

private:
    int frameCounter_ = 0;
    Animation* logo_;
};
