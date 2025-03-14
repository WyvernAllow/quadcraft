#include "chunk.h"

static inline int get_index(int x, int y, int z) {
    return (z * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + x;
}

static inline bool in_bounds(int x, int y, int z) {
    return (x >= 0 && x < CHUNK_SIZE) && (y >= 0 && y < CHUNK_SIZE) &&
           (z >= 0 && z < CHUNK_SIZE);
}

block_type get_block(const chunk *chunk, int x, int y, int z) {
    if (!in_bounds(x, y, z)) {
        return BLOCK_AIR;
    }

    return chunk->blocks[get_index(x, y, z)];
}

void set_block(chunk *chunk, int x, int y, int z, block_type type) {
    if (!in_bounds(x, y, z)) {
        return;
    }

    chunk->blocks[get_index(x, y, z)] = type;
}
