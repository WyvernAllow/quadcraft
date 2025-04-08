#include "direction.hpp"

#include <cassert>

glm::vec3 direction_to_vec3(direction dir) {
    assert(dir < direction::NUM_DIRECTIONS);

    switch (dir) {
    case direction::POS_X:
        return glm::vec3(1.0f, 0.0f, 0.0f);
    case direction::POS_Y:
        return glm::vec3(0.0f, 1.0f, 0.0f);
    case direction::POS_Z:
        return glm::vec3(0.0f, 0.0f, 1.0f);
    case direction::NEG_X:
        return glm::vec3(-1.0f, 0.0f, 0.0f);
    case direction::NEG_Y:
        return glm::vec3(0.0f, -1.0f, 0.0f);
    case direction::NEG_Z:
        return glm::vec3(0.0f, 0.0f, -1.0f);
    }

    return glm::vec3(0);
}

glm::ivec3 direction_to_ivec3(direction dir) {
    assert(dir < direction::NUM_DIRECTIONS);

    switch (dir) {
    case direction::POS_X:
        return glm::ivec3(1, 0, 0);
    case direction::POS_Y:
        return glm::ivec3(0, 1, 0);
    case direction::POS_Z:
        return glm::ivec3(0, 0, 1);
    case direction::NEG_X:
        return glm::ivec3(-1, 0, 0);
    case direction::NEG_Y:
        return glm::ivec3(0, -1, 0);
    case direction::NEG_Z:
        return glm::ivec3(0, 0, -1);
    }

    return glm::ivec3(0);
}