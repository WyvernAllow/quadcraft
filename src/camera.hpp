#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class camera {
public:
    glm::vec3 position;

    float pitch;
    float yaw;

    float fov;
    float aspect;
    float near;
    float far;

    camera(float fov, float aspect, float near = 0.01f, float far = 1000.0f);

public:
    void update();

    const glm::vec3& forward() const;
    const glm::vec3& up() const;
    const glm::vec3& right() const;

    const glm::mat4& view() const;
    const glm::mat4& proj() const;

private:
    glm::vec3 m_forward;
    glm::vec3 m_up;
    glm::vec3 m_right;

    glm::mat4 m_view;
    glm::mat4 m_proj;
};