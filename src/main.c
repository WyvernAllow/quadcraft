#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "chunk.h"
#include "lmath.h"

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

typedef struct chunk_vertex {
    vec3 position;
    vec3 normal;

    vec2 atlas_offset;
} chunk_vertex;

vec3 normal_lut[DIR_COUNT] = {
    [DIR_POS_X] = (vec3){1.0f, 0.0f, 0.0f},
    [DIR_POS_Y] = (vec3){0.0f, 1.0f, 0.0f},
    [DIR_POS_Z] = (vec3){0.0f, 0.0f, 1.0f},
    [DIR_NEG_X] = (vec3){-1.0f, 0.0f, 0.0f},
    [DIR_NEG_Y] = (vec3){0.0f, -1.0f, 0.0f},
    [DIR_NEG_Z] = (vec3){0.0f, 0.0f, -1.0f},
};

SDL_GPUShader *load_shader(SDL_GPUDevice *device, const char *filename,
                           SDL_GPUShaderStage stage, size_t num_uniforms,
                           size_t num_samplers) {
    size_t code_size;
    const uint8_t *code = SDL_LoadFile(filename, &code_size);
    if (!code) {
        SDL_Log("SDL_LoadFile failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_GPUShaderCreateInfo shader_info = {0};
    shader_info.code = code;
    shader_info.code_size = code_size;
    shader_info.entrypoint = "main";
    shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    shader_info.stage = stage;
    shader_info.num_uniform_buffers = num_uniforms;
    shader_info.num_samplers = num_samplers;

    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_info);
    if (!shader) {
        SDL_Log("SDL_CreateGPUShader failed: %s\n", SDL_GetError());
        return NULL;
    }

    return shader;
}

SDL_GPUGraphicsPipeline *create_pipeline(SDL_GPUDevice *device,
                                         SDL_Window *window) {
    SDL_GPUShader *vert = load_shader(device, "res/shaders/main.vert.spv",
                                      SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
    if (!vert) {
        SDL_Log("Failed to load vertex shader");
        return NULL;
    }

    SDL_GPUShader *frag = load_shader(device, "res/shaders/main.frag.spv",
                                      SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 1);
    if (!frag) {
        SDL_Log("Failed to load fragment shader");
        return NULL;
    }

    SDL_GPUGraphicsPipelineCreateInfo info = {0};

    SDL_GPUColorTargetDescription color_target_descriptions[] = {
        (SDL_GPUColorTargetDescription){
            .format = SDL_GetGPUSwapchainTextureFormat(device, window)}};

    info.target_info.num_color_targets = ARRAY_LEN(color_target_descriptions);
    info.target_info.color_target_descriptions = color_target_descriptions;

    info.target_info.has_depth_stencil_target = true;
    info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    info.depth_stencil_state.enable_depth_test = true;
    info.depth_stencil_state.enable_depth_write = true;
    info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

    info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[] = {
        (SDL_GPUVertexBufferDescription){
            .slot = 0,
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
            .pitch = sizeof(chunk_vertex),
        }};

    SDL_GPUVertexAttribute vertex_attributes[] = {
        /* Position attribute */
        (SDL_GPUVertexAttribute){.buffer_slot = 0,
                                 .location = 0,
                                 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                                 .offset = offsetof(chunk_vertex, position)},

        /* Normal attribute */
        (SDL_GPUVertexAttribute){.buffer_slot = 0,
                                 .location = 1,
                                 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                                 .offset = offsetof(chunk_vertex, normal)},

        /* Atlas offset attribute */
        (SDL_GPUVertexAttribute){
            .buffer_slot = 0,
            .location = 2,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = offsetof(chunk_vertex, atlas_offset)}};

    info.vertex_input_state = (SDL_GPUVertexInputState){
        .vertex_buffer_descriptions = vertex_buffer_descriptions,
        .num_vertex_buffers = ARRAY_LEN(vertex_buffer_descriptions),
        .vertex_attributes = vertex_attributes,
        .num_vertex_attributes = ARRAY_LEN(vertex_attributes)};

    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.vertex_shader = vert;
    info.fragment_shader = frag;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline *pipeline =
        SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline) {
        SDL_Log("SDL_CreateGPUGraphicsPipeline failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_ReleaseGPUShader(device, vert);
    SDL_ReleaseGPUShader(device, frag);

    return pipeline;
}

SDL_GPUTexture *create_depth_texture(SDL_GPUDevice *device, uint32_t width,
                                     uint32_t height) {
    SDL_GPUTextureCreateInfo info = {0};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

    return SDL_CreateGPUTexture(device, &info);
}

#define MAX_QUADS (CHUNK_VOLUME * 6)
#define MAX_VERTS (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)

typedef struct chunk_mesh {
    chunk_vertex *vertices;
    size_t vertex_count;

    uint16_t *indices;
    size_t index_count;

    SDL_GPUBuffer *vertex_buffer;
    SDL_GPUBuffer *index_buffer;
    SDL_GPUTransferBuffer *transfer_buffer;
} chunk_mesh;

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

void add_quad(chunk_mesh *mesh, vec3 offset, vec3 normal, vec2 atlas_offset) {
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

void add_cube(const chunk *chunk, chunk_mesh *mesh, int x, int y, int z) {
    for (direction d = 0; d < DIR_COUNT; d++) {
        vec3 position = {x, y, z};
        vec3 normal = normal_lut[d];
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

void chunk_mesh_generate(const chunk *chunk, chunk_mesh *mesh,
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

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window =
        SDL_CreateWindow("Quadcraft", 800, 450, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetWindowRelativeMouseMode(window, true);

    SDL_Surface *surface = SDL_LoadBMP("res/textures/atlas.bmp");
    if (!surface) {
        SDL_Log("SDL_LoadBMP failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);

    SDL_GPUDevice *device =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!device) {
        SDL_Log("SDL_CreateGPUDevice failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("SDL_ClaimWindowForGPUDevice failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetGPUSwapchainParameters(device, window,
                                  SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                  SDL_GPU_PRESENTMODE_MAILBOX);

    SDL_GPUGraphicsPipeline *pipeline = create_pipeline(device, window);
    if (!pipeline) {
        SDL_Log("Failed to create pipeline\n");
        return EXIT_FAILURE;
    }

    chunk chunk;
    chunk_init(&chunk);

    chunk_mesh mesh;
    chunk_mesh_init(&mesh, device);

    camera cam;
    camera_init(&cam, 90.0f, 800.0f / 450.0f, 0.01f, 1000.0f);

    SDL_GPUSampler *sampler = SDL_CreateGPUSampler(
        device, &(SDL_GPUSamplerCreateInfo){
                    .min_filter = SDL_GPU_FILTER_NEAREST,
                    .mag_filter = SDL_GPU_FILTER_NEAREST,
                    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
                    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                });

    SDL_GPUTexture *texture = SDL_CreateGPUTexture(
        device, &(SDL_GPUTextureCreateInfo){
                    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                    .width = surface->w,
                    .height = surface->h,
                    .layer_count_or_depth = 1,
                    .num_levels = 1,
                    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                });

    SDL_GPUTransferBuffer *texture_trans_buf = SDL_CreateGPUTransferBuffer(
        device, &(SDL_GPUTransferBufferCreateInfo){
                    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                    .size = surface->w * surface->h * 4});

    void *texels = SDL_MapGPUTransferBuffer(device, texture_trans_buf, false);
    memcpy(texels, surface->pixels, surface->w * surface->h * 4);
    SDL_UnmapGPUTransferBuffer(device, texture_trans_buf);

    SDL_GPUCommandBuffer *upload_cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmdbuf);

    SDL_UploadToGPUTexture(
        copy_pass,
        &(SDL_GPUTextureTransferInfo){
            .transfer_buffer = texture_trans_buf,
            .offset = 0,
        },
        &(SDL_GPUTextureRegion){
            .texture = texture, .w = surface->w, .h = surface->h, .d = 1},
        false);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmdbuf);
    SDL_ReleaseGPUTransferBuffer(device, texture_trans_buf);

    SDL_GPUTexture *depth = create_depth_texture(device, 800, 450);

    float current_time = SDL_GetTicks() / 1000.0f;
    float last_time = current_time;

    bool fullscreen = false;
    if (fullscreen) {
        SDL_SetWindowFullscreen(window, true);
    } else {
        SDL_SetWindowFullscreen(window, false);
    }

    uint32_t old_w = 800;
    uint32_t old_h = 450;

    block_type type = BLOCK_STONE;

    bool is_running = true;
    while (is_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                is_running = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                cam.pitch -= e.motion.yrel * 0.4f;
                cam.yaw += e.motion.xrel * 0.4f;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (e.key.scancode == SDL_SCANCODE_F11) {
                    fullscreen = !fullscreen;

                    if (fullscreen) {
                        SDL_SetWindowFullscreen(window, true);
                    } else {
                        SDL_SetWindowFullscreen(window, false);
                    }
                }

                if (e.key.scancode >= 30 && e.key.scancode < 39) {
                    int block_type = e.key.scancode - 30;

                    type = block_type;

                    SDL_Log("Selected block type: %i\n", type);
                }
            }
        }

        current_time = SDL_GetTicks() / 1000.0f;
        float delta_time = current_time - last_time;
        last_time = current_time;

        const bool *keys = SDL_GetKeyboardState(NULL);

        vec3 wishdir = {0};
        if (keys[SDL_SCANCODE_W]) {
            wishdir = vec3_add(wishdir, cam.forward);
        }

        if (keys[SDL_SCANCODE_S]) {
            wishdir = vec3_sub(wishdir, cam.forward);
        }

        if (keys[SDL_SCANCODE_A]) {
            wishdir = vec3_sub(wishdir, cam.right);
        }

        if (keys[SDL_SCANCODE_D]) {
            wishdir = vec3_add(wishdir, cam.right);
        }

        float_t x, y;
        SDL_MouseButtonFlags mouse_state = SDL_GetMouseState(&x, &y);

        if (mouse_state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
            vec3 cam_forward = vec3_add(cam.position, cam.forward);
            int x_pos = (int)cam_forward.x;
            int y_pos = (int)cam_forward.y;
            int z_pos = (int)cam_forward.z;

            chunk_set_block(&chunk, x_pos, y_pos, z_pos, BLOCK_AIR);
        }

        if (mouse_state & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
            vec3 cam_forward = vec3_add(cam.position, cam.forward);
            int x_pos = (int)cam_forward.x;
            int y_pos = (int)cam_forward.y;
            int z_pos = (int)cam_forward.z;

            chunk_set_block(&chunk, x_pos, y_pos, z_pos, type);
        }

        cam.position = vec3_add(
            cam.position, vec3_scale(vec3_norm(wishdir), 6.0f * delta_time));

        camera_update(&cam);

        SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(device);
        if (!cmdbuf) {
            SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s\n", SDL_GetError());
            return EXIT_FAILURE;
        }

        SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmdbuf);

        if (chunk.is_dirty) {
            chunk_mesh_generate(&chunk, &mesh, device);
            chunk_mesh_upload(&mesh, copy_pass);
            chunk.is_dirty = false;
        }

        SDL_EndGPUCopyPass(copy_pass);

        uint32_t new_w;
        uint32_t new_h;
        SDL_GPUTexture *swapchain_tex;
        if (!SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &swapchain_tex,
                                            &new_w, &new_h)) {
            SDL_Log("SDL_AcquireGPUSwapchainTexture failed: %s\n",
                    SDL_GetError());
            return EXIT_FAILURE;
        }

        if (old_h != new_h || new_w != old_w) {
            SDL_ReleaseGPUTexture(device, depth);

            depth = create_depth_texture(device, new_w, new_h);
            SDL_Log("Creating new depth texture");
        }

        old_w = new_w;
        old_h = new_h;

        if (!swapchain_tex) {
            SDL_CancelGPUCommandBuffer(cmdbuf);
            continue;
        }

        SDL_GPUColorTargetInfo color_target_info = {
            .texture = swapchain_tex,
            .clear_color = (SDL_FColor){0.015f, 0.015f, 0.015f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        SDL_GPUDepthStencilTargetInfo depth_target_info = {
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
            .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
            .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
            .texture = depth,
            .cycle = true};

        SDL_PushGPUVertexUniformData(cmdbuf, 0, cam.view_proj.m,
                                     sizeof(float) * 16);

        SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
            cmdbuf, &color_target_info, 1, &depth_target_info);

        SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
        SDL_BindGPUVertexBuffers(
            render_pass, 0,
            &(SDL_GPUBufferBinding){.buffer = mesh.vertex_buffer, .offset = 0},
            1);

        SDL_BindGPUIndexBuffer(
            render_pass,
            &(SDL_GPUBufferBinding){.buffer = mesh.index_buffer, .offset = 0},
            SDL_GPU_INDEXELEMENTSIZE_16BIT);

        SDL_BindGPUFragmentSamplers(render_pass, 0,
                                    &(SDL_GPUTextureSamplerBinding){
                                        .texture = texture, .sampler = sampler},
                                    1);

        SDL_DrawGPUIndexedPrimitives(render_pass, mesh.index_count, 1, 0, 0, 0);

        SDL_EndGPURenderPass(render_pass);

        SDL_SubmitGPUCommandBuffer(cmdbuf);
    }

    SDL_ReleaseGPUTexture(device, depth);
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    chunk_mesh_free(&mesh, device);
    SDL_ReleaseGPUTexture(device, texture);
    SDL_ReleaseGPUSampler(device, sampler);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
