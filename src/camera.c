#include "camera.h"

#include <math.h>

void camera_init(camera *cam, float fov_deg, float aspect, float near,
                 float far) {
    cam->position.x = 0.0f;
    cam->position.y = 0.0f;
    cam->position.z = 0.0f;

    cam->pitch = 0.0f;
    cam->yaw = -90.0f;

    cam->fov_deg = fov_deg;
    cam->aspect = aspect;
    cam->near = near;
    cam->far = far;

    camera_update(cam);
}

void camera_update(camera *cam) {
    /* Pitch values of 90 degrees up or down cause gimbal lock. */
    cam->pitch = clamp(cam->pitch, -89.999f, 89.999f);

    vec3 forward;
    forward.x = cosf(deg_to_rad(cam->yaw)) * cosf(deg_to_rad(cam->pitch));
    forward.y = sinf(deg_to_rad(cam->pitch));
    forward.z = sinf(deg_to_rad(cam->yaw)) * cosf(deg_to_rad(cam->pitch));

    cam->forward = vec3_norm(forward);
    cam->right = vec3_norm(vec3_cross(cam->forward, (vec3){0.0f, 1.0f, 0.0f}));
    cam->up = vec3_norm(vec3_cross(cam->right, cam->forward));

    mat4_perspective(&cam->proj, deg_to_rad(cam->fov_deg), cam->aspect,
                     cam->near, cam->far);
    mat4_lookat(&cam->view, cam->position,
                vec3_add(cam->position, cam->forward), cam->up);
    mat4_mul(&cam->view_proj, &cam->proj, &cam->view);
}
