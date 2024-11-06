#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

#include <stdbool.h>
#include "lmath.h"

typedef enum block_type {
    BLOCK_AIR,
    BLOCK_STONE,
    BLOCK_DIRT,
    BLOCK_GRASS,
    BLOCK_LOG,
    BLOCK_BRICK,
    BLOCK_WOOD,

    BLOCK_COUNT,
} block_type;

typedef enum direction {
    DIR_POS_X,
    DIR_POS_Y,
    DIR_POS_Z,
    DIR_NEG_X,
    DIR_NEG_Y,
    DIR_NEG_Z,

    DIR_COUNT,
} direction;

typedef struct block_properties {
    bool is_transparent;
    vec2 atlas_offsets[DIR_COUNT];
} block_properties;

const block_properties *get_block_properties(block_type type);

#endif /* BLOCK_TYPE_H */
