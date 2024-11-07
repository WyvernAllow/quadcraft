#include "chunk.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "FastNoiseLite.h"

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

    fnl_state noise = fnlCreateState();
    noise.seed = time(NULL);
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;

    memset(chunk->blocks, 0, CHUNK_VOLUME * sizeof(block_type));

    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                float v = (fnlGetNoise2D(&noise, (float)x * CHUNK_SIZE * 0.5f,
                                         (float)z * CHUNK_SIZE * 0.5f) +
                           1.0) /
                          2.0f;
                int height = (v * CHUNK_SIZE / 4);

                if (y > height) {
                    chunk->blocks[get_index(x, y, z)] = BLOCK_AIR;
                    continue;
                }

                if (y == height) {
                    chunk->blocks[get_index(x, y, z)] = BLOCK_GRASS;
                } else if (y < height) {
                    chunk->blocks[get_index(x, y, z)] = BLOCK_DIRT;
                }
            }
        }
    }

    chunk->is_dirty = true;
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
    chunk->is_dirty = true;
}
