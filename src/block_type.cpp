#include "block_type.hpp"

static block_properties
    block_properties_array[(size_t)block_type::NUM_BLOCK_TYPES];

void init_block_properties() {
    block_properties_array[(size_t)(block_type::AIR)] = {false};
    block_properties_array[(size_t)(block_type::STONE)] = {true};
}

block_properties& get_block_properties(block_type type) {
    return block_properties_array[(size_t)type];
}