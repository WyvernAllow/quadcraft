#include "lmath.h"

#include <float.h>
#include <math.h>
#include <string.h>

float to_rad(float deg) {
    return deg * (M_PI / 180.0f);
}

float to_deg(float rad) {
    return rad * (180.0f / M_PI);
}

float clampf(float value, float min, float max) {
    return fmaxf(min, fminf(value, max));
}

vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
    };
}

vec3 vec3_sub(vec3 a, vec3 b) {
    return (vec3){
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
    };
}

vec3 vec3_scale(vec3 v, float s) {
    return (vec3){
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s,
    };
}

vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x,
    };
}

float vec3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_len_squared(vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vec3_len(vec3 v) {
    return sqrtf(vec3_len_squared(v));
}

vec3 vec3_normalized(vec3 v) {
    float len = vec3_len(v);
    if (len > FLT_EPSILON) {
        return vec3_scale(v, 1.0f / len);
    }

    return (vec3){0.0f, 0.0f, 0.0f};
}

void mat4_identity(mat4 *out) {
    memset(out, 0, sizeof(mat4));

    out->data[0] = 1.0f;
    out->data[5] = 1.0f;
    out->data[10] = 1.0f;
    out->data[15] = 1.0f;
}

void mat4_perspective(mat4 *out, float fov, float aspect, float near,
                      float far) {
    mat4_identity(out);
    float inv_tan_half_fov = 1.0f / tanf(fov * 0.5f);

    out->data[0] = inv_tan_half_fov / aspect;
    out->data[5] = inv_tan_half_fov;
    out->data[10] = (near + far) / (near - far);
    out->data[11] = -1.0f;
    out->data[14] = (2.0f * near * far) / (near - far);
    out->data[15] = 0.0f;
}

void mat4_lookat(mat4 *out, vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_normalized(vec3_sub(center, eye));
    vec3 s = vec3_normalized(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);

    mat4_identity(out);
    out->data[0] = s.x;
    out->data[1] = u.x;
    out->data[2] = -f.x;
    out->data[4] = s.y;
    out->data[5] = u.y;
    out->data[6] = -f.y;
    out->data[8] = s.z;
    out->data[9] = u.z;
    out->data[10] = -f.z;

    out->data[12] = -vec3_dot(s, eye);
    out->data[13] = -vec3_dot(u, eye);
    out->data[14] = vec3_dot(f, eye);
}
