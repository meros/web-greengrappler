#pragma once
#include <string>
#include <map>

class GameState {
public:
    static void clear();
    static int getInt(const std::string& key);
    static void put(const std::string& key, int value);
    static bool isSavePresent();
    static void loadFromFile();
    static void saveToFile();

private:
    static inline std::map<std::string, std::string> dataMap_;
    static std::string getSavePath();
};
