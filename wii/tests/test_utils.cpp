#include "utils.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static constexpr float EPS = 0.001f;
static bool near(float a, float b) { return std::fabs(a - b) < EPS; }

static void test_lerp_middle() {
    assert(near(lerp(0.0f, 10.0f, 0.5f), 5.0f));
}

static void test_lerp_start() {
    assert(near(lerp(0.0f, 10.0f, 0.0f), 0.0f));
}

static void test_lerp_end() {
    assert(near(lerp(0.0f, 10.0f, 1.0f), 10.0f));
}

static void test_lerp_clamp_low() {
    assert(near(lerp(0.0f, 10.0f, -0.5f), 0.0f));
}

static void test_lerp_clamp_high() {
    assert(near(lerp(0.0f, 10.0f, 1.5f), 10.0f));
}

static void test_lerp_quarter() {
    assert(near(lerp(0.0f, 100.0f, 0.25f), 25.0f));
}

static void test_lerpInt() {
    assert(lerpInt(0.0f, 10.0f, 0.5f) == 5);
    assert(lerpInt(0.0f, 10.0f, 0.35f) == 3);
    assert(lerpInt(0.0f, 10.0f, 0.99f) == 9);
}

static void test_sincos() {
    Vec2 r = sincos(0.0f);
    assert(near(r.x, 1.0f)); // cos(0) = 1
    assert(near(r.y, 0.0f)); // sin(0) = 0

    Vec2 r2 = sincos(static_cast<float>(M_PI / 2));
    assert(near(r2.x, 0.0f)); // cos(pi/2) = 0
    assert(near(r2.y, 1.0f)); // sin(pi/2) = 1
}

static void test_makeRGB() {
    uint32_t color = makeRGB(255, 128, 0);
    assert((color & 0xFF000000) == 0xFF000000); // Alpha = 255
    assert(((color >> 16) & 0xFF) == 255);      // R
    assert(((color >> 8) & 0xFF) == 128);       // G
    assert((color & 0xFF) == 0);                // B
}

int main() {
    test_lerp_middle();
    test_lerp_start();
    test_lerp_end();
    test_lerp_clamp_low();
    test_lerp_clamp_high();
    test_lerp_quarter();
    test_lerpInt();
    test_sincos();
    test_makeRGB();
    std::printf("All utils tests passed.\n");
    return 0;
}
