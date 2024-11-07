#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "lmath.h"

typedef struct chunk_vertex {
    vec3 position;
    vec3 normal;

    vec2 atlas_offset;
} chunk_vertex;

typedef struct chunk_mesh {
    chunk_vertex *vertices;
    size_t vertex_count;

    uint16_t *indices;
    size_t index_count;

    SDL_GPUBuffer *vertex_buffer;
    SDL_GPUBuffer *index_buffer;
    SDL_GPUTransferBuffer *transfer_buffer;
} chunk_mesh;

void chunk_mesh_init(chunk_mesh *mesh, SDL_GPUDevice *device);
void chunk_mesh_free(chunk_mesh *mesh, SDL_GPUDevice *device);
void chunk_mesh_upload(chunk_mesh *mesh, SDL_GPUCopyPass *copy_pass);
void chunk_mesh_remesh(chunk_mesh *mesh, const chunk *chunk,
                       SDL_GPUDevice *device);

#endif /* CHUNK_MESH_H */
