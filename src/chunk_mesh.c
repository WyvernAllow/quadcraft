#include "chunk_mesh.h"

#include "chunk.h"

#define MAX_QUADS (CHUNK_VOLUME * 6)
#define MAX_VERTS (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)

static const vec3 NORMAL_LUT[DIR_COUNT] = {
    [DIR_POS_X] = (vec3){1.0f, 0.0f, 0.0f},
    [DIR_POS_Y] = (vec3){0.0f, 1.0f, 0.0f},
    [DIR_POS_Z] = (vec3){0.0f, 0.0f, 1.0f},
    [DIR_NEG_X] = (vec3){-1.0f, 0.0f, 0.0f},
    [DIR_NEG_Y] = (vec3){0.0f, -1.0f, 0.0f},
    [DIR_NEG_Z] = (vec3){0.0f, 0.0f, -1.0f},
};

static void add_quad(chunk_mesh *mesh, vec3 offset, vec3 normal,
                     vec2 atlas_offset) {
    vec3 up = {0.0f, 1.0f, 0.0f};
    if (fabsf(normal.y) > 0.9f) {
        up = (vec3){1.0f, 0.0f, 0.0f};
    }

    vec3 tangent = vec3_norm(vec3_cross(up, normal));
    vec3 bitangent = vec3_cross(normal, tangent);

    vec3 positions[4] = {
        vec3_add(offset, vec3_add(vec3_scale(tangent, -0.5f),
                                  vec3_scale(bitangent, -0.5f))),
        vec3_add(offset, vec3_add(vec3_scale(tangent, 0.5f),
                                  vec3_scale(bitangent, -0.5f))),
        vec3_add(offset, vec3_add(vec3_scale(tangent, 0.5f),
                                  vec3_scale(bitangent, 0.5f))),
        vec3_add(offset, vec3_add(vec3_scale(tangent, -0.5f),
                                  vec3_scale(bitangent, 0.5f))),
    };

    size_t base_index = mesh->vertex_count;
    for (int i = 0; i < 4; i++) {
        mesh->vertices[mesh->vertex_count].position =
            vec3_add(positions[i], (vec3){0.5f, 0.5f, 0.5f});
        mesh->vertices[mesh->vertex_count].normal = normal;
        mesh->vertices[mesh->vertex_count].atlas_offset = atlas_offset;
        mesh->vertex_count++;
    }

    mesh->indices[mesh->index_count++] = base_index + 0;
    mesh->indices[mesh->index_count++] = base_index + 1;
    mesh->indices[mesh->index_count++] = base_index + 2;
    mesh->indices[mesh->index_count++] = base_index + 0;
    mesh->indices[mesh->index_count++] = base_index + 2;
    mesh->indices[mesh->index_count++] = base_index + 3;
}

static void add_cube(const chunk *chunk, chunk_mesh *mesh, int x, int y,
                     int z) {
    for (direction d = 0; d < DIR_COUNT; d++) {
        vec3 position = {x, y, z};
        vec3 normal = NORMAL_LUT[d];
        vec3 adjacent = vec3_add(position, normal);

        block_type compare_block = chunk_get_block(
            chunk, (int)adjacent.x, (int)adjacent.y, (int)adjacent.z);

        block_type current_block = chunk_get_block(chunk, x, y, z);
        if (current_block != BLOCK_AIR) {
            const block_properties *properties =
                get_block_properties(current_block);

            if (compare_block == BLOCK_AIR) {
                add_quad(mesh, vec3_add(vec3_scale(normal, 0.5), position),
                         normal, properties->atlas_offsets[d]);
            }
        }
    }
}

void chunk_mesh_init(chunk_mesh *mesh, SDL_GPUDevice *device) {
    mesh->vertices = NULL;
    mesh->vertex_count = 0;
    mesh->indices = NULL;
    mesh->index_count = 0;

    SDL_GPUBufferCreateInfo vert_buffer_info = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(chunk_vertex) * MAX_VERTS,
    };

    SDL_GPUBufferCreateInfo index_buffer_info = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(uint16_t) * MAX_INDICES,
    };

    SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size =
            sizeof(chunk_vertex) * MAX_VERTS + sizeof(uint16_t) * MAX_INDICES,
    };

    mesh->vertex_buffer = SDL_CreateGPUBuffer(device, &vert_buffer_info);
    if (!mesh->vertex_buffer) {
        SDL_Log("Failed to create chunk vertex buffer: %s", SDL_GetError());
    }

    mesh->index_buffer = SDL_CreateGPUBuffer(device, &index_buffer_info);
    if (!mesh->index_buffer) {
        SDL_Log("Failed to create chunk index buffer: %s", SDL_GetError());
    }

    mesh->transfer_buffer =
        SDL_CreateGPUTransferBuffer(device, &transfer_buffer_info);
    if (!mesh->transfer_buffer) {
        SDL_Log("Failed to create chunk transfer buffer: %s", SDL_GetError());
    }
}

void chunk_mesh_free(chunk_mesh *mesh, SDL_GPUDevice *device) {
    SDL_ReleaseGPUTransferBuffer(device, mesh->transfer_buffer);
    SDL_ReleaseGPUBuffer(device, mesh->index_buffer);
    SDL_ReleaseGPUBuffer(device, mesh->vertex_buffer);
}

void chunk_mesh_upload(chunk_mesh *mesh, SDL_GPUCopyPass *copy_pass) {
    SDL_GPUTransferBufferLocation vert_src = {
        .transfer_buffer = mesh->transfer_buffer, .offset = 0};

    SDL_GPUBufferRegion vert_dest = {
        .buffer = mesh->vertex_buffer,
        .offset = 0,
        .size = sizeof(chunk_vertex) * mesh->vertex_count,
    };

    SDL_GPUTransferBufferLocation index_src = {
        .transfer_buffer = mesh->transfer_buffer,
        .offset = sizeof(chunk_vertex) * MAX_VERTS};

    SDL_GPUBufferRegion index_dest = {
        .buffer = mesh->index_buffer,
        .offset = 0,
        .size = sizeof(uint16_t) * mesh->index_count,
    };

    SDL_UploadToGPUBuffer(copy_pass, &vert_src, &vert_dest, false);
    SDL_UploadToGPUBuffer(copy_pass, &index_src, &index_dest, false);
}

void chunk_mesh_remesh(chunk_mesh *mesh, const chunk *chunk,
                       SDL_GPUDevice *device) {
    uint8_t *data =
        SDL_MapGPUTransferBuffer(device, mesh->transfer_buffer, false);

    mesh->vertices = (chunk_vertex *)data;
    mesh->indices = (uint16_t *)(data + sizeof(chunk_vertex) * MAX_VERTS);
    mesh->vertex_count = 0;
    mesh->index_count = 0;

    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                add_cube(chunk, mesh, x, y, z);
            }
        }
    }

    SDL_UnmapGPUTransferBuffer(device, mesh->transfer_buffer);
}
