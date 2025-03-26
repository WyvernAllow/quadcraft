#include "math2d.h"

#include <float.h>
#include <math.h>

float to_radians(float deg) {
    return deg * (PI / 180.0f);
}

float to_degrees(float rad) {
    return rad * (180.0f / PI);
}

float clamp(float value, float min, float max) {
    if (min > max) {
        float tmp = min;
        min = max;
        max = tmp;
    }

    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

float lerp(float a, float b, float t) {
    return (1.0f - t) * a + t * b;
}

float lerp_smooth(float a, float b, float dt, float h) {
    if (h < FLT_EPSILON) {
        return b;
    }
    return b + (a - b) * exp2f(-dt / h);
}

vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

vec2 vec2_sub(vec2 a, vec2 b) {
    return (vec2){
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
}

vec2 vec2_scale(vec2 v, float s) {
    return (vec2){
        .x = v.x * s,
        .y = v.y * s,
    };
}

float vec2_dot(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float vec2_len_squared(vec2 v) {
    return v.x * v.x + v.y * v.y;
}

float vec2_len(vec2 v) {
    return sqrtf(vec2_len_squared(v));
}

vec2 vec2_normalized(vec2 v) {
    float len = vec2_len(v);
    if (len < FLT_EPSILON) {
        return (vec2){
            .x = 0.0f,
            .y = 0.0f,
        };
    }

    return vec2_scale(v, 1.0f / len);
}

vec2 vec2_from_angle(float theta) {
    return (vec2){
        .x = cosf(theta),
        .y = sinf(theta),
    };
}

vec2 vec2_clamp(vec2 value, vec2 min, vec2 max) {
    return (vec2){
        .x = clamp(value.x, min.x, max.x),
        .y = clamp(value.y, min.y, max.y),
    };
}

vec2 vec2_lerp(vec2 a, vec2 b, float t) {
    return (vec2){
        .x = lerp(a.x, b.x, t),
        .y = lerp(a.y, b.y, t),
    };
}

vec2 vec2_lerp_smooth(vec2 a, vec2 b, float dt, float h) {
    return (vec2){
        .x = lerp_smooth(a.x, b.x, dt, h),
        .y = lerp_smooth(a.y, b.y, dt, h),
    };
}

ivec2 ivec2_add(ivec2 a, ivec2 b) {
    return (ivec2){
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

ivec2 ivec2_sub(ivec2 a, ivec2 b) {
    return (ivec2){
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
}
