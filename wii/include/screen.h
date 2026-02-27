#pragma once
#include <vector>
#include <SDL2/SDL.h>

class Screen {
public:
    virtual ~Screen() = default;
    virtual void onDraw(SDL_Renderer* renderer) = 0;
    virtual void onLogic() = 0;

    virtual void onEntered() {}
    virtual void onExited() {}
    virtual bool onEnter(SDL_Renderer* renderer) { (void)renderer; return true; }
    virtual bool onExit(SDL_Renderer* renderer) { (void)renderer; return true; }

    void exit();
    bool isTop() const;
};

class ScreenManager {
public:
    static void add(Screen* screen);
    static void exit(Screen* screen);
    static Screen* getTop();
    static bool isEmpty();
    static void onLogic();
    static void draw(SDL_Renderer* renderer);
    static void clear();

private:
    static inline std::vector<Screen*> stack_;
    static inline Screen* screenToEnter_ = nullptr;
    static inline Screen* screenToExit_ = nullptr;
};
