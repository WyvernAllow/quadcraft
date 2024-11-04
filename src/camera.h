#ifndef CAMERA_H
#define CAMERA_H

#include "lmath.h"

typedef struct camera {
    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;
    float pitch;
    float yaw;

    float fov_deg;
    float aspect;
    float near;
    float far;

    mat4 view;
    mat4 proj;
    mat4 view_proj;
} camera;

void camera_init(camera *cam, float fov_deg, float aspect, float near, float far);
void camera_update(camera *cam);

#endif /* CAMERA_H */