#pragma once
#include "utils.h"

class PlayerSkill {
public:
    static float get() { return skill_; }

    static void playerDidSomethingClever(float howClever, float howImportant) {
        if (howClever > skill_) {
            skill_ = lerp(skill_, howClever, howImportant);
        }
    }

    static void playerDidSomethingStupid(float howClever, float howImportant) {
        if (howClever < skill_) {
            skill_ = lerp(skill_, howClever, howImportant);
        }
    }

    static void reset() { skill_ = 0.5f; }

private:
    static inline float skill_ = 0.5f;
};
