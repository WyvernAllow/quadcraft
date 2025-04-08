#include "chunk.hpp"

chunk::chunk() {
    m_blocks.fill(block_type::STONE);
    is_dirty = true;
}

bool chunk::in_bounds(int local_x, int local_y, int local_z) {
    return local_x >= 0 && local_x < SIZE_X && local_y >= 0 &&
           local_y < SIZE_Y && local_z >= 0 && local_z < SIZE_Z;
}

bool chunk::in_bounds(glm::ivec3 local_position) {
    return in_bounds(local_position.x, local_position.y, local_position.z);
}

size_t chunk::get_index(int local_x, int local_y, int local_z) {
    return local_x + local_y * SIZE_X + local_z * SIZE_X * SIZE_Y;
}

void chunk::set_block(size_t index, block_type type) {
    if (index >= m_blocks.size()) {
        return;
    }

    m_blocks[index] = type;
    is_dirty = true;
}

void chunk::set_block(int local_x, int local_y, int local_z, block_type type) {
    if (!in_bounds(local_x, local_y, local_z)) {
        return;
    }

    set_block(get_index(local_x, local_y, local_z), type);
}

void chunk::set_block(glm::ivec3 local_position, block_type type) {
    set_block(local_position.x, local_position.y, local_position.z, type);
}

block_type chunk::get_block(size_t index) const {
    if (index >= m_blocks.size()) {
        return block_type::AIR;
    }

    return m_blocks[index];
}

block_type chunk::get_block(int local_x, int local_y, int local_z) const {
    if (!in_bounds(local_x, local_y, local_z)) {
        return block_type::AIR;
    }

    return get_block(get_index(local_x, local_y, local_z));
}

block_type chunk::get_block(glm::ivec3 local_position) const {
    return get_block(local_position.x, local_position.y, local_position.z);
}

bool chunk::dirty() const {
    return is_dirty;
}

void chunk::mark_dirty() {
    is_dirty = true;
}

void chunk::mark_clean() {
    is_dirty = false;
}