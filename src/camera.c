#include "camera.h"

#include <math.h>
#include <string.h>

#include "lmath.h"

void init_camera(camera *cam, float fov, float aspect, float near, float far) {
    memset(cam, 0, sizeof(camera));
    cam->pitch = 0.0f;
    cam->yaw = -M_PI / 2.0f;

    cam->fov = fov;
    cam->aspect = aspect;
    cam->near = near;
    cam->far = far;

    update_camera(cam);
}

void update_camera(camera *cam) {
    mat4_perspective(&cam->proj, cam->fov, cam->aspect, cam->near, cam->far);

    /* Pitch values of +-90 degrees cause the camera to flip. */
    cam->pitch = clampf(cam->pitch, -M_PI / 2.0 + 1e-5, M_PI / 2.0 - 1e-5);

    vec3 forward = {
        .x = cosf(cam->yaw) * cosf(cam->pitch),
        .y = sinf(cam->pitch),
        .z = sinf(cam->yaw) * cosf(cam->pitch),
    };

    const vec3 WORLD_UP = {0.0f, 1.0f, 0.0f};

    cam->forward = vec3_normalized(forward);
    cam->right = vec3_normalized(vec3_cross(cam->forward, WORLD_UP));
    cam->up = vec3_normalized(vec3_cross(cam->right, cam->forward));

    mat4_lookat(&cam->view, cam->position,
                vec3_add(cam->position, cam->forward), cam->up);
}
