#pragma once
#include "resource.h"
#include <vector>
#include <string>
#include <cmath>

class Animation {
public:
    Animation(const std::string& filename, int numFrames = -1);

    int getFrameWidth() const { return frameWidth_; }
    int getFrameHeight() const { return frameHeight_; }
    GameImage getFrame(int index) const;
    GameImage getGameImage() const { return frames_.empty() ? GameImage() : frames_[0]; }

    void drawFrame(SDL_Renderer* renderer, int frame, int x, int y,
                   bool hFlip = false, bool vFlip = false) const;

private:
    int frameWidth_;
    int frameHeight_;
    std::vector<GameImage> frames_;
};
