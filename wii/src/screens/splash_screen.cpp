#include "screens/splash_screen.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "constants.h"

SplashScreen::SplashScreen() {
    logo_ = Resource::getAnimation("data/images/logo.bmp", 1);
}

void SplashScreen::onDraw(SDL_Renderer* renderer) {
    int yOffset = -160 + frameCounter_;
    if (yOffset == 0) Sound::playSample("data/sounds/boot");
    if (yOffset > 0) yOffset = 0;

    SDL_SetRenderDrawColor(renderer, 231, 215, 156, 255);
    SDL_Rect bg = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &bg);

    logo_->drawFrame(renderer, 0,
        160 - logo_->getFrameWidth() / 2,
        120 - logo_->getFrameHeight() / 2 + yOffset);
}

void SplashScreen::onLogic() {
    if (frameCounter_ > 250) {
        exit();
        return;
    }
    frameCounter_++;
}
