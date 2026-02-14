#include "media/animation.h"
#include "constants.h"
#include <cstdlib>

Animation::Animation(const std::string& filename, int numFrames) {
    GameImage allFrames = Resource::getBitmap(filename);
    frameHeight_ = allFrames.height;

    if (numFrames > 0) {
        frameWidth_ = allFrames.width / numFrames;
    } else {
        frameWidth_ = frameHeight_;
        numFrames = (frameWidth_ > 0) ? allFrames.width / frameWidth_ : 1;
    }

    if (numFrames <= 0) numFrames = 1;

    for (int i = 0; i < numFrames; i++) {
        frames_.push_back(allFrames.subImage(i * frameWidth_, 0, frameWidth_, frameHeight_));
    }
}

GameImage Animation::getFrame(int index) const {
    if (frames_.empty()) return {};
    return frames_[std::abs(index) % static_cast<int>(frames_.size())];
}

void Animation::drawFrame(SDL_Renderer* renderer, int frame, int x, int y,
                          bool hFlip, bool vFlip) const {
    if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT ||
        x + frameWidth_ < 0 || y + frameHeight_ < 0) return;

    GameImage img = getFrame(frame);
    if (!img.texture) return;

    SDL_Rect src = {img.sx, img.sy, img.sw, img.sh};
    SDL_Rect dst = {x, y, img.sw, img.sh};

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (hFlip) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    if (vFlip) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);

    SDL_RenderCopyEx(renderer, img.texture, &src, &dst, 0.0, nullptr, flip);
}
