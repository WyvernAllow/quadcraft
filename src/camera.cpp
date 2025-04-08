#include "camera.hpp"

camera::camera(float fov, float aspect, float near, float far)
    : fov(fov),
      aspect(aspect),
      near(near),
      far(far),
      pitch(0.0),
      yaw(-glm::half_pi<float>()) {
    update();
}

void camera::update() {
    pitch = glm::clamp(pitch, -glm::half_pi<float>() + 0.01f,
                       glm::half_pi<float>() - 0.01f);

    glm::vec3 forward(0.0);
    forward.x = glm::cos(yaw) * glm::cos(pitch);
    forward.y = glm::sin(pitch);
    forward.z = glm::sin(yaw) * glm::cos(pitch);

    m_forward = glm::normalize(forward);
    m_right =
        glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));

    m_view = glm::lookAt(position, position + m_forward, m_up);
    m_proj = glm::perspective(fov, aspect, near, far);
}

const glm::vec3& camera::forward() const {
    return m_forward;
}

const glm::vec3& camera::up() const {
    return m_up;
}

const glm::vec3& camera::right() const {
    return m_right;
}

const glm::mat4& camera::view() const {
    return m_view;
}

const glm::mat4& camera::proj() const {
    return m_proj;
}