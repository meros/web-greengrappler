#include "collision.h"
#include <cassert>
#include <cstdio>

static void test_collides_overlapping() {
    SimpleCollidable a(0, 0, 10, 10);
    SimpleCollidable b(5, 5, 15, 15);
    assert(collides(a, b));
    assert(collides(b, a));
}

static void test_collides_not_overlapping() {
    SimpleCollidable a(0, 0, 10, 10);
    SimpleCollidable b(20, 20, 30, 30);
    assert(!collides(a, b));
    assert(!collides(b, a));
}

static void test_collides_touching_edges() {
    SimpleCollidable a(0, 0, 10, 10);
    SimpleCollidable b(10, 0, 20, 10);
    // Touching at edge — right == left, so right < left is false, collision IS detected
    assert(collides(a, b));
}

static void test_collides_contained() {
    SimpleCollidable outer(0, 0, 100, 100);
    SimpleCollidable inner(25, 25, 75, 75);
    assert(collides(outer, inner));
    assert(collides(inner, outer));
}

static void test_collides_rect() {
    SimpleCollidable a(5, 5, 15, 15);
    assert(collidesRect(a, 0, 0, 10, 10));
    assert(!collidesRect(a, 20, 20, 30, 30));
}

static void test_collides_horizontal_separation() {
    SimpleCollidable a(0, 0, 10, 10);
    SimpleCollidable b(11, 0, 20, 10);
    assert(!collides(a, b));
}

static void test_collides_vertical_separation() {
    SimpleCollidable a(0, 0, 10, 10);
    SimpleCollidable b(0, 11, 10, 20);
    assert(!collides(a, b));
}

static void test_collides_single_point() {
    SimpleCollidable a(5, 5, 5, 5);
    SimpleCollidable b(5, 5, 5, 5);
    assert(collides(a, b)); // Same point
}

int main() {
    test_collides_overlapping();
    test_collides_not_overlapping();
    test_collides_touching_edges();
    test_collides_contained();
    test_collides_rect();
    test_collides_horizontal_separation();
    test_collides_vertical_separation();
    test_collides_single_point();
    std::printf("All collision tests passed.\n");
    return 0;
}
