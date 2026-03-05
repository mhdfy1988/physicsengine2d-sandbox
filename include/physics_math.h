#ifndef PHYSICS_MATH_H
#define PHYSICS_MATH_H

#include <math.h>

#ifndef PI
#define PI 3.14159265359f
#endif

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float m[2][2];  /* 2x2 matrix */
} Mat2;

/* Vector operations */
Vec2 vec2(float x, float y);
Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_scale(Vec2 v, float s);
Vec2 vec2_negate(Vec2 v);
float vec2_dot(Vec2 a, Vec2 b);
float vec2_cross(Vec2 a, Vec2 b);
float vec2_length_sq(Vec2 v);
float vec2_length(Vec2 v);
Vec2 vec2_normalize(Vec2 v);
Vec2 vec2_rotate(Vec2 v, float angle);

/* Matrix operations */
Mat2 mat2(float a, float b, float c, float d);
Mat2 mat2_identity(void);
Mat2 mat2_rotation(float angle);
Vec2 mat2_mul_vec2(Mat2 m, Vec2 v);
Mat2 mat2_mul(Mat2 a, Mat2 b);
Mat2 mat2_transpose(Mat2 m);
Mat2 mat2_invert(Mat2 m);

/* Utility helpers */
float clamp(float value, float min, float max);
float min_f(float a, float b);
float max_f(float a, float b);
int is_equal(float a, float b, float epsilon);

#endif
