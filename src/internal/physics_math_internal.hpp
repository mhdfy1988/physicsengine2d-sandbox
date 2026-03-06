#ifndef PHYSICS_MATH_INTERNAL_H
#define PHYSICS_MATH_INTERNAL_H

#include "../../include/physics_math.hpp"

static inline Vec2 physics_cross_scalar_vec(float s, Vec2 v) {
    return vec2(-s * v.y, s * v.x);
}

#endif
