#pragma once
#include <glm/vec3.hpp>

enum class direction : uint8_t {
    POS_X,
    POS_Y,
    POS_Z,
    NEG_X,
    NEG_Y,
    NEG_Z,

    NUM_DIRECTIONS,
};

glm::vec3 direction_to_vec3(direction dir);
glm::ivec3 direction_to_ivec3(direction dir);