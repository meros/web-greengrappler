#pragma once
#include <string>
#include <vector>
#include <SDL2/SDL_mixer.h>

class Music {
public:
    static void playSong(const std::string& path);
    static void play();
    static void stop();
    static void pushSong();
    static void popSong();
    static void update();
    static void shutdown();

private:
    static std::string mapPath(const std::string& path);
    static inline Mix_Music* currentMusic_ = nullptr;
    static inline std::string currentPath_;
    static inline std::vector<std::string> songStack_;
};
