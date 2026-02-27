#include "player_skill.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static constexpr float EPS = 0.001f;
static bool near(float a, float b) { return std::fabs(a - b) < EPS; }

static void test_initial_value() {
    PlayerSkill::reset();
    assert(near(PlayerSkill::get(), 0.5f));
}

static void test_clever_increases_skill() {
    PlayerSkill::reset();
    float before = PlayerSkill::get();
    PlayerSkill::playerDidSomethingClever(1.0f, 0.5f);
    assert(PlayerSkill::get() > before);
}

static void test_clever_only_if_above() {
    PlayerSkill::reset(); // 0.5
    PlayerSkill::playerDidSomethingClever(0.3f, 0.5f); // 0.3 < 0.5, should not change
    assert(near(PlayerSkill::get(), 0.5f));
}

static void test_stupid_decreases_skill() {
    PlayerSkill::reset();
    float before = PlayerSkill::get();
    PlayerSkill::playerDidSomethingStupid(0.0f, 0.5f);
    assert(PlayerSkill::get() < before);
}

static void test_stupid_only_if_below() {
    PlayerSkill::reset(); // 0.5
    PlayerSkill::playerDidSomethingStupid(0.7f, 0.5f); // 0.7 > 0.5, should not change
    assert(near(PlayerSkill::get(), 0.5f));
}

static void test_reset() {
    PlayerSkill::playerDidSomethingClever(1.0f, 1.0f);
    PlayerSkill::reset();
    assert(near(PlayerSkill::get(), 0.5f));
}

static void test_clever_with_lerp() {
    PlayerSkill::reset(); // 0.5
    PlayerSkill::playerDidSomethingClever(1.0f, 0.1f);
    // lerp(0.5, 1.0, 0.1) = 0.5 + 0.5*0.1 = 0.55
    assert(near(PlayerSkill::get(), 0.55f));
}

static void test_stupid_with_lerp() {
    PlayerSkill::reset(); // 0.5
    PlayerSkill::playerDidSomethingStupid(0.0f, 0.1f);
    // lerp(0.5, 0.0, 0.1) = 0.5 + (-0.5)*0.1 = 0.45
    assert(near(PlayerSkill::get(), 0.45f));
}

int main() {
    test_initial_value();
    test_clever_increases_skill();
    test_clever_only_if_above();
    test_stupid_decreases_skill();
    test_stupid_only_if_below();
    test_reset();
    test_clever_with_lerp();
    test_stupid_with_lerp();
    std::printf("All PlayerSkill tests passed.\n");
    return 0;
}
