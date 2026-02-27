#include "vec2.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static constexpr float EPS = 0.001f;
static bool near(float a, float b) { return std::fabs(a - b) < EPS; }

static void test_default_construction() {
    Vec2 v;
    assert(v.x == 0.0f && v.y == 0.0f);
}

static void test_value_construction() {
    Vec2 v(3.0f, 4.0f);
    assert(v.x == 3.0f && v.y == 4.0f);
}

static void test_addition() {
    Vec2 a(1.0f, 2.0f);
    Vec2 b(3.0f, 4.0f);
    Vec2 c = a + b;
    assert(c.x == 4.0f && c.y == 6.0f);
}

static void test_subtraction() {
    Vec2 a(5.0f, 7.0f);
    Vec2 b(2.0f, 3.0f);
    Vec2 c = a - b;
    assert(c.x == 3.0f && c.y == 4.0f);
}

static void test_scalar_multiply() {
    Vec2 v(3.0f, 4.0f);
    Vec2 r = v * 2.0f;
    assert(r.x == 6.0f && r.y == 8.0f);
}

static void test_scalar_divide() {
    Vec2 v(6.0f, 8.0f);
    Vec2 r = v / 2.0f;
    assert(r.x == 3.0f && r.y == 4.0f);
}

static void test_length() {
    Vec2 v(3.0f, 4.0f);
    assert(near(v.length(), 5.0f));
}

static void test_length_sq() {
    Vec2 v(3.0f, 4.0f);
    assert(near(v.lengthSq(), 25.0f));
}

static void test_normalize() {
    Vec2 v(3.0f, 4.0f);
    v.normalize();
    assert(near(v.length(), 1.0f));
    assert(near(v.x, 0.6f));
    assert(near(v.y, 0.8f));
}

static void test_normalized_returns_copy() {
    Vec2 v(3.0f, 4.0f);
    Vec2 n = v.normalized();
    assert(near(n.length(), 1.0f));
    assert(near(v.length(), 5.0f)); // Original unchanged
}

static void test_dot() {
    Vec2 a(1.0f, 0.0f);
    Vec2 b(0.0f, 1.0f);
    assert(near(a.dot(b), 0.0f));

    Vec2 c(2.0f, 3.0f);
    Vec2 d(4.0f, 5.0f);
    assert(near(c.dot(d), 23.0f));
}

static void test_is_zero() {
    Vec2 zero;
    assert(zero.isZero());

    Vec2 nonzero(1.0f, 0.0f);
    assert(!nonzero.isZero());
}

static void test_length_compare() {
    Vec2 v(3.0f, 4.0f);
    assert(v.lengthCompare(5.0f) < EPS && v.lengthCompare(5.0f) > -EPS);
    assert(v.lengthCompare(4.0f) > 0);
    assert(v.lengthCompare(6.0f) < 0);
}

static void test_compound_assignment() {
    Vec2 v(1.0f, 2.0f);
    v += Vec2(3.0f, 4.0f);
    assert(v.x == 4.0f && v.y == 6.0f);

    v -= Vec2(1.0f, 1.0f);
    assert(v.x == 3.0f && v.y == 5.0f);

    v *= 2.0f;
    assert(v.x == 6.0f && v.y == 10.0f);
}

static void test_normalize_zero_vector() {
    Vec2 v;
    v.normalize();
    assert(v.x == 0.0f && v.y == 0.0f); // Should not crash
}

int main() {
    test_default_construction();
    test_value_construction();
    test_addition();
    test_subtraction();
    test_scalar_multiply();
    test_scalar_divide();
    test_length();
    test_length_sq();
    test_normalize();
    test_normalized_returns_copy();
    test_dot();
    test_is_zero();
    test_length_compare();
    test_compound_assignment();
    test_normalize_zero_vector();
    std::printf("All Vec2 tests passed.\n");
    return 0;
}
