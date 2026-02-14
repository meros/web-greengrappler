#include "game_state.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

static void test_default_value() {
    GameState::clear();
    assert(GameState::getInt("nonexistent") == 0);
}

static void test_put_and_get() {
    GameState::clear();
    GameState::put("test_key", 42);
    assert(GameState::getInt("test_key") == 42);
}

static void test_overwrite() {
    GameState::clear();
    GameState::put("key", 1);
    GameState::put("key", 2);
    assert(GameState::getInt("key") == 2);
}

static void test_clear() {
    GameState::put("key", 42);
    GameState::clear();
    assert(GameState::getInt("key") == 0);
}

static void test_save_and_load() {
    GameState::clear();
    GameState::put("level1", 1);
    GameState::put("coins", 50);
    GameState::saveToFile();

    // Reset in-memory state without deleting file
    GameState::put("level1", 0);
    GameState::put("coins", 0);
    assert(GameState::getInt("level1") == 0);

    GameState::loadFromFile();
    assert(GameState::getInt("level1") == 1);
    assert(GameState::getInt("coins") == 50);

    // Clean up
    std::remove("greengrappler.sav");
    GameState::clear();
}

static void test_is_save_present() {
    GameState::clear();
    assert(!GameState::isSavePresent());

    GameState::put("test", 1);
    GameState::saveToFile(); // This sets save_present = 1
    // Reset in-memory
    GameState::put("save_present", 0);
    assert(!GameState::isSavePresent());

    GameState::loadFromFile();
    assert(GameState::isSavePresent());

    // Clean up
    std::remove("greengrappler.sav");
    GameState::clear();
}

int main() {
    test_default_value();
    test_put_and_get();
    test_overwrite();
    test_clear();
    test_save_and_load();
    test_is_save_present();
    std::printf("All GameState tests passed.\n");
    return 0;
}
