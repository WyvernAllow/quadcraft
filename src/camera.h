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

    float fov;
    float aspect;
    float near;
    float far;

    mat4 view;
    mat4 proj;
} camera;

void init_camera(camera *camera, float fov, float aspect, float near,
                 float far);
void update_camera(camera *camera);

#endif /* CAMERA_H */
