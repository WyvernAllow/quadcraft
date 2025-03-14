#ifndef CHUNK_H
#define CHUNK_H

#include "block_type.h"

#define CHUNK_SIZE 32
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

typedef struct chunk {
    block_type blocks[CHUNK_VOLUME];
} chunk;

block_type get_block(const chunk *chunk, int x, int y, int z);
void set_block(chunk *chunk, int x, int y, int z, block_type type);

#endif /* CHUNK_H */
