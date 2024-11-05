#include "chunk.h"

#include <stdbool.h>

static bool in_bounds(int x, int y, int z) {
    return (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 &&
            z < CHUNK_SIZE);
}

static size_t get_index(int x, int y, int z) {
    return (size_t)(x + y * CHUNK_SIZE + z * (CHUNK_SIZE * CHUNK_SIZE));
}

void chunk_init(chunk *chunk) {
    chunk->x = 0;
    chunk->y = 0;

    for (size_t i = 0; i < CHUNK_VOLUME; i++) {
        chunk->blocks[i] = BLOCK_STONE;
    }
}

block_type chunk_get_block(const chunk *chunk, int x, int y, int z) {
    if (!in_bounds(x, y, z)) {
        return BLOCK_AIR;
    }

    return chunk->blocks[get_index(x, y, z)];
}

void chunk_set_block(chunk *chunk, int x, int y, int z, block_type type) {
    if (!in_bounds(x, y, z)) {
        return;
    }

    chunk->blocks[get_index(x, y, z)] = type;
}
