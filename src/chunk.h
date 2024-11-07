#ifndef CHUNK_H
#define CHUNK_H
#include "block_type.h"

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

typedef struct chunk {
    int x;
    int y;
    block_type blocks[CHUNK_VOLUME];
    
    bool is_dirty;
} chunk;

void chunk_init(chunk *chunk);
block_type chunk_get_block(const chunk *chunk, int x, int y, int z);
void chunk_set_block(chunk *chunk, int x, int y, int z, block_type type);

#endif /* CHUNK_H */
