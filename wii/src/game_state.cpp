#include "game_state.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

#ifdef HW_RVL
#include <fat.h>
static const char* SAVE_DIR = "sd:/apps/greengrappler/";
#else
static const char* SAVE_DIR = "";
#endif

std::string GameState::getSavePath() {
    std::string path = SAVE_DIR;
    path += "greengrappler.sav";
    return path;
}

void GameState::clear() {
    dataMap_.clear();
    // Also delete save file
    std::remove(getSavePath().c_str());
}

int GameState::getInt(const std::string& key) {
    auto it = dataMap_.find(key);
    if (it == dataMap_.end()) return 0;
    return std::atoi(it->second.c_str());
}

void GameState::put(const std::string& key, int value) {
    dataMap_[key] = std::to_string(value);
}

bool GameState::isSavePresent() {
    return getInt("save_present") == 1;
}

void GameState::loadFromFile() {
    dataMap_.clear();
    std::ifstream file(getSavePath());
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            dataMap_[key] = val;
        }
    }
}

void GameState::saveToFile() {
    dataMap_["save_present"] = "1";
    std::ofstream file(getSavePath());
    if (!file.is_open()) return;

    for (auto& [key, val] : dataMap_) {
        file << key << "=" << val << "\n";
    }
}
