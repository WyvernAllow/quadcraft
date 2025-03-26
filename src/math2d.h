#ifndef MATH2D_H
#define MATH2D_H

#define PI 3.14159265358979323846

float to_radians(float deg);
float to_degrees(float rad);
float clamp(float value, float min, float max);
float lerp(float a, float b, float t);
float lerp_smooth(float a, float b, float dt, float h);

typedef struct vec2 {
    float x;
    float y;
} vec2;

vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 v, float s);
float vec2_dot(vec2 a, vec2 b);
float vec2_len_squared(vec2 v);
float vec2_len(vec2 v);
vec2 vec2_normalized(vec2 v);
vec2 vec2_from_angle(float theta);
vec2 vec2_clamp(vec2 value, vec2 min, vec2 max);
vec2 vec2_lerp(vec2 a, vec2 b, float t);
vec2 vec2_lerp_smooth(vec2 a, vec2 b, float dt, float h);

typedef struct ivec2 {
    int x;
    int y;
} ivec2;

ivec2 ivec2_add(ivec2 a, ivec2 b);
ivec2 ivec2_sub(ivec2 a, ivec2 b);

#endif /* MATH2D_H */
