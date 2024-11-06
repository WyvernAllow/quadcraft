#include "block_type.h"

const block_properties block_table[BLOCK_COUNT] = {
    [BLOCK_AIR] =
        {
            .is_transparent = true,
            .atlas_offsets = {0},
        },

    [BLOCK_STONE] =
        {
            .is_transparent = false,
            .atlas_offsets =
                {
                    [DIR_POS_X] = {0, 0},
                    [DIR_POS_Y] = {0, 0},
                    [DIR_POS_Z] = {0, 0},
                    [DIR_NEG_X] = {0, 0},
                    [DIR_NEG_Y] = {0, 0},
                    [DIR_NEG_Z] = {0, 0},
                },
        },

    [BLOCK_DIRT] =
        {
            .is_transparent = false,
            .atlas_offsets =
                {
                    [DIR_POS_X] = {1, 0},
                    [DIR_POS_Y] = {1, 0},
                    [DIR_POS_Z] = {1, 0},
                    [DIR_NEG_X] = {1, 0},
                    [DIR_NEG_Y] = {1, 0},
                    [DIR_NEG_Z] = {1, 0},
                },
        },

    [BLOCK_GRASS] =
        {
            .is_transparent = false,
            .atlas_offsets =
                {
                    [DIR_POS_X] = {2, 0},
                    [DIR_POS_Y] = {3, 0},
                    [DIR_POS_Z] = {2, 0},
                    [DIR_NEG_X] = {2, 0},
                    [DIR_NEG_Y] = {1, 0},
                    [DIR_NEG_Z] = {2, 0},
                },
        },

    [BLOCK_LOG] =
        {
            .is_transparent = false,
            .atlas_offsets =
                {
                    [DIR_POS_X] = {4, 0},
                    [DIR_POS_Y] = {5, 0},
                    [DIR_POS_Z] = {4, 0},
                    [DIR_NEG_X] = {4, 0},
                    [DIR_NEG_Y] = {5, 0},
                    [DIR_NEG_Z] = {4, 0},
                },
        },

    [BLOCK_BRICK] =
        {
            .is_transparent = false,
            .atlas_offsets =
                {
                    [DIR_POS_X] = {6, 0},
                    [DIR_POS_Y] = {6, 0},
                    [DIR_POS_Z] = {6, 0},
                    [DIR_NEG_X] = {6, 0},
                    [DIR_NEG_Y] = {6, 0},
                    [DIR_NEG_Z] = {6, 0},
                },
        },

    [BLOCK_WOOD] =
        {
            .is_transparent = false,
            .atlas_offsets =
                {
                    [DIR_POS_X] = {7, 0},
                    [DIR_POS_Y] = {7, 0},
                    [DIR_POS_Z] = {7, 0},
                    [DIR_NEG_X] = {7, 0},
                    [DIR_NEG_Y] = {7, 0},
                    [DIR_NEG_Z] = {7, 0},
                },
        },
};

const block_properties *get_block_properties(block_type type) {
    return &block_table[type];
}
