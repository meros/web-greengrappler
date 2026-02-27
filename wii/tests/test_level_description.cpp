#include "level_description.h"
#include <cassert>
#include <cstdio>

static void test_default_construction() {
    LevelDescription desc;
    assert(desc.name.empty());
    assert(desc.levelFile.empty());
    assert(desc.frameIndex == 0);
    assert(desc.musicFile.empty());
}

static void test_value_construction() {
    LevelDescription desc("TEST LEVEL", "data/rooms/test.txt", 5, "data/music/test.xm");
    assert(desc.name == "TEST LEVEL");
    assert(desc.levelFile == "data/rooms/test.txt");
    assert(desc.frameIndex == 5);
    assert(desc.musicFile == "data/music/test.xm");
}

static void test_copy() {
    LevelDescription a("A", "b.txt", 1, "c.xm");
    LevelDescription b = a;
    assert(b.name == "A");
    assert(b.frameIndex == 1);
}

int main() {
    test_default_construction();
    test_value_construction();
    test_copy();
    std::printf("All LevelDescription tests passed.\n");
    return 0;
}
