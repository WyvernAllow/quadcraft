#ifndef LMATH_H
#define LMATH_H

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_scale(vec3 v, float s);
float vec3_dot(vec3 a, vec3 b);
vec3 vec3_cross(vec3 a, vec3 b);
float vec3_len(vec3 v);
vec3 vec3_norm(vec3 v);

typedef struct mat4 {
    float m[4 * 4];
} mat4;

void mat4_identity(mat4 *out);
void mat4_perspective(mat4 *out, float fov, float aspect, float near, float far);
void mat4_lookat(mat4 *out, vec3 eye, vec3 center, vec3 up);
void mat4_mul(mat4 *out, mat4 *a, mat4 *b);

#endif /* LMATH_H */
