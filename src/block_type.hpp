#pragma once
#include <cstdint>
#include <string_view>

enum class block_type : uint8_t {
    AIR,
    STONE,

    NUM_BLOCK_TYPES,
};

struct block_properties {
    bool is_transparent;
};

void init_block_properties();
block_properties& get_block_properties(block_type type);