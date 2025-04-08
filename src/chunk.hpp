#pragma once
#include <array>
#include <glm/glm.hpp>

#include "block_type.hpp"

class chunk {
public:
    static constexpr size_t SIZE_X = 16;
    static constexpr size_t SIZE_Y = 16;
    static constexpr size_t SIZE_Z = 16;
    static constexpr size_t VOLUME = SIZE_X * SIZE_Y * SIZE_Z;

public:
    chunk();

    static bool in_bounds(int local_x, int local_y, int local_z);
    static bool in_bounds(glm::ivec3 local_position);
    static size_t get_index(int local_x, int local_y, int local_z);

    void set_block(size_t index, block_type type);
    void set_block(int local_x, int local_y, int local_z, block_type type);
    void set_block(glm::ivec3 local_position, block_type type);

    block_type get_block(size_t index) const;
    block_type get_block(int local_x, int local_y, int local_z) const;
    block_type get_block(glm::ivec3 local_position) const;

    bool dirty() const;

    void mark_dirty();
    void mark_clean();

private:
    bool is_dirty;
    std::array<block_type, VOLUME> m_blocks;
};