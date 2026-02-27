#include "screens/end_screen.h"
#include "resource.h"
#include "media/font.h"
#include "media/sound.h"
#include "media/music.h"
#include "input.h"
#include "constants.h"

EndScreen::EndScreen() {
    font_ = Resource::getFont("data/images/font.bmp");
}

void EndScreen::onDraw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 57, 56, 41, 255);
    SDL_Rect bg = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &bg);

    if (frameCounter_ < 0) return;

    if (frameCounter_ < GREEN_PEACE_DURATION) {
        font_->draw(renderer, "THE LAST REACTOR IS IN CAPTIVITY", 50, 110);
        font_->draw(renderer, "THE GALAXY IS AT GREEN PEACE", 60, 120);
    } else {
        int o = creditsOffset_;
        font_->draw(renderer, "CREDITS", 50, o);
        font_->draw(renderer, "PROGRAMMING", 50, 30 + o);
        font_->draw(renderer, "   OLOF NAESSEN", 50, 40 + o);
        font_->draw(renderer, "   PER LARSSON", 50, 50 + o);
        font_->draw(renderer, "   ALEXANDER SCHRAB ", 50, 60 + o);
        font_->draw(renderer, "GRAPHICS", 50, 90 + o);
        font_->draw(renderer, "   OLOF NAESSEN", 50, 100 + o);
        font_->draw(renderer, "   TIMUR KONDRAKOV", 50, 110 + o);
        font_->draw(renderer, "   PER LARSSON", 50, 120 + o);
        font_->draw(renderer, "MUSIC", 50, 150 + o);
        font_->draw(renderer, "   OLOF NAESSEN", 50, 160 + o);
        font_->draw(renderer, "VOICE ACTING", 50, 190 + o);
        font_->draw(renderer, "   PER LARSSON", 50, 200 + o);
        font_->draw(renderer, "NOT SHOWING UP FOR THE COMPO", 50, 230 + o);
        font_->draw(renderer, "   TED STEEN", 50, 240 + o);
        font_->draw(renderer, "THIS GAME WAS DONE", 50, 270 + o);
        font_->draw(renderer, "FOR THE SPEEDHACK 11", 50, 280 + o);
        font_->draw(renderer, "COMPETITION", 50, 290 + o);
        font_->draw(renderer, "THANK YOU ALLEGRO.CC", 50, 300 + o);
        font_->draw(renderer, "FOR A GREAT COMPETITION!", 50, 310 + o);
        font_->draw(renderer, "THANK YOU THORBJORN LINDEIJER", 50, 340 + o);
        font_->draw(renderer, "FOR THE TILED EDITOR!", 50, 350 + o);
        font_->draw(renderer, "THANK YOU ANDERS STENBERG", 50, 380 + o);
        font_->draw(renderer, "FOR YOUR TILE RAYCASTING ALGORITHM!", 50, 390 + o);
        font_->draw(renderer, "AND THANKS FOR PLAYING", 50, 420 + o);
        font_->draw(renderer, "OUR GAME!", 50, 430 + o);

        if (creditsOffset_ < -450) {
            font_->drawCenter(renderer, "THE END", 0, 0, 320, 240);
        }
    }
}

void EndScreen::onLogic() {
    frameCounter_++;
    if (frameCounter_ == 0) Sound::playSample("data/sounds/green_peace");
    if (frameCounter_ == GREEN_PEACE_DURATION) {
        Music::playSong("data/music/intro2.xm");
    } else if (frameCounter_ > GREEN_PEACE_DURATION && frameCounter_ % 4 == 0) {
        creditsOffset_--;
    }
    if (Input::isPressed(Button::EXIT)) exit();
}
