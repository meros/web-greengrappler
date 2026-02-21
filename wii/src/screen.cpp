#include "screen.h"
#include <algorithm>
#include <cstdio>

void Screen::exit() {
    ScreenManager::exit(this);
}

bool Screen::isTop() const {
    return ScreenManager::getTop() == this;
}

void ScreenManager::add(Screen* screen) {
    stack_.push_back(screen);
    screenToEnter_ = screen;
}

void ScreenManager::exit(Screen* screen) {
    if (getTop() != screen) return;
    if (screenToExit_ != nullptr) return;
    screenToExit_ = screen;
}

Screen* ScreenManager::getTop() {
    return stack_.empty() ? nullptr : stack_.back();
}

bool ScreenManager::isEmpty() {
    return stack_.empty();
}

void ScreenManager::onLogic() {
    if (screenToExit_ == nullptr && screenToEnter_ == nullptr) {
        auto* top = getTop();
        if (top) top->onLogic();
    }
}

void ScreenManager::draw(SDL_Renderer* renderer) {
    static int drawCount = 0;
    if (drawCount < 5) {
        std::fprintf(stderr, "DEBUG: ScreenManager::draw #%d (exit=%p enter=%p top=%p)\n",
            drawCount, (void*)screenToExit_, (void*)screenToEnter_, (void*)getTop());
        std::fflush(stderr);
    }

    if (screenToExit_ == nullptr && screenToEnter_ == nullptr) {
        auto* top = getTop();
        if (top) {
            if (drawCount < 5) { std::fprintf(stderr, "DEBUG: Drawing top screen\n"); std::fflush(stderr); }
            top->onDraw(renderer);
            if (drawCount < 5) { std::fprintf(stderr, "DEBUG: Top screen drawn\n"); std::fflush(stderr); }
        }
    }

    if (screenToExit_ != nullptr) {
        bool exitDone = screenToExit_->onExit(renderer);
        if (!exitDone) return;
        auto it = std::find(stack_.begin(), stack_.end(), screenToExit_);
        if (it != stack_.end()) {
            stack_.erase(it);
        }
        screenToExit_->onExited();
        delete screenToExit_;
        screenToExit_ = nullptr;
        if (getTop() != nullptr) screenToEnter_ = getTop();
    }

    if (screenToEnter_ != nullptr) {
        if (drawCount < 5) { std::fprintf(stderr, "DEBUG: Entering screen\n"); std::fflush(stderr); }
        bool enterDone = screenToEnter_->onEnter(renderer);
        if (!enterDone) return;
        screenToEnter_->onEntered();
        screenToEnter_ = nullptr;
        if (drawCount < 5) { std::fprintf(stderr, "DEBUG: Screen entered\n"); std::fflush(stderr); }
    }
    drawCount++;
}

void ScreenManager::clear() {
    for (auto* s : stack_) delete s;
    stack_.clear();
    screenToEnter_ = nullptr;
    screenToExit_ = nullptr;
}
