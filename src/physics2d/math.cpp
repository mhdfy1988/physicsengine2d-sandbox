#include <math.h>
#include <stdlib.h>
#include "../include/physics_math.hpp"

// ===== Vector Operations =====
Vec2 vec2(float x, float y) {
    Vec2 v = {x, y};
    return v;
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2(a.x + b.x, a.y + b.y);
}

Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return vec2(a.x - b.x, a.y - b.y);
}

Vec2 vec2_scale(Vec2 v, float s) {
    return vec2(v.x * s, v.y * s);
}

Vec2 vec2_negate(Vec2 v) {
    return vec2(-v.x, -v.y);
}

float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float vec2_cross(Vec2 a, Vec2 b) {
    return a.x * b.y - a.y * b.x;
}

float vec2_length_sq(Vec2 v) {
    return v.x * v.x + v.y * v.y;
}

float vec2_length(Vec2 v) {
    return sqrtf(vec2_length_sq(v));
}

Vec2 vec2_normalize(Vec2 v) {
    float len = vec2_length(v);
    if (len < 1e-6f) {
        return vec2(0, 0);
    }
    return vec2_scale(v, 1.0f / len);
}

Vec2 vec2_rotate(Vec2 v, float angle) {
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    return vec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a);
}

// ===== Matrix Operations =====
Mat2 mat2(float a, float b, float c, float d) {
    Mat2 m = {{{a, b}, {c, d}}};
    return m;
}

Mat2 mat2_identity(void) {
    return mat2(1, 0, 0, 1);
}

Mat2 mat2_rotation(float angle) {
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    return mat2(cos_a, -sin_a, sin_a, cos_a);
}

Vec2 mat2_mul_vec2(Mat2 m, Vec2 v) {
    return vec2(m.m[0][0] * v.x + m.m[0][1] * v.y,
                m.m[1][0] * v.x + m.m[1][1] * v.y);
}

Mat2 mat2_mul(Mat2 a, Mat2 b) {
    return mat2(
        a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0],
        a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1],
        a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0],
        a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1]
    );
}

Mat2 mat2_transpose(Mat2 m) {
    return mat2(m.m[0][0], m.m[1][0], m.m[0][1], m.m[1][1]);
}

Mat2 mat2_invert(Mat2 m) {
    float det = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
    if (fabs(det) < 1e-6f) {
        return mat2_identity();
    }
    float inv_det = 1.0f / det;
    return mat2(
        m.m[1][1] * inv_det,
        -m.m[0][1] * inv_det,
        -m.m[1][0] * inv_det,
        m.m[0][0] * inv_det
    );
}

// ===== Utility Functions =====
float clamp(float value, float min, float max) {
    if (!isfinite(value)) return min;
    if (!isfinite(min)) min = 0.0f;
    if (!isfinite(max)) max = min;
    if (max < min) {
        float t = min;
        min = max;
        max = t;
    }
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float min_f(float a, float b) {
    return a < b ? a : b;
}

float max_f(float a, float b) {
    return a > b ? a : b;
}

int is_equal(float a, float b, float epsilon) {
    return fabs(a - b) < epsilon;
}
