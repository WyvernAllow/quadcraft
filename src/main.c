#include <SDL3/SDL.h>
#include <stdlib.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

typedef struct vertex {
    float position[3];
    float color[3];
} vertex;

SDL_GPUShader *load_shader(SDL_GPUDevice *device, const char *filename,
                           SDL_GPUShaderStage stage) {
    size_t code_size;
    const uint8_t *code = SDL_LoadFile(filename, &code_size);
    if (!code) {
        SDL_Log("SDL_LoadFile failed: %s\n", SDL_GetError());
        return NULL;
    }

    const SDL_GPUShaderCreateInfo shader_info = {
        .code = code,
        .code_size = code_size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
    };

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
                                      SDL_GPU_SHADERSTAGE_VERTEX);
    if (!vert) {
        SDL_Log("Failed to load vertex shader");
        return NULL;
    }

    SDL_GPUShader *frag = load_shader(device, "res/shaders/main.frag.spv",
                                      SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!frag) {
        SDL_Log("Failed to load fragment shader");
        return NULL;
    }

    const SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
        .target_info = {.num_color_targets = 1,
                        .color_target_descriptions =
                            (SDL_GPUColorTargetDescription[]){
                                (SDL_GPUColorTargetDescription){
                                    .format = SDL_GetGPUSwapchainTextureFormat(
                                        device, window)}}},

        .vertex_input_state =
            (SDL_GPUVertexInputState){
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions =
                    (SDL_GPUVertexBufferDescription[]){
                        (SDL_GPUVertexBufferDescription){
                            .slot = 0,
                            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                            .instance_step_rate = 0,
                            .pitch = sizeof(vertex),
                        }},
                .num_vertex_attributes = 2,
                .vertex_attributes =
                    (SDL_GPUVertexAttribute[]){
                        (SDL_GPUVertexAttribute){
                            .buffer_slot = 0,
                            .location = 0,
                            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                            .offset = offsetof(vertex, position),
                        },

                        (SDL_GPUVertexAttribute){
                            .buffer_slot = 0,
                            .location = 1,
                            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                            .offset = offsetof(vertex, color),
                        },
                    }},

        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vert,
        .fragment_shader = frag,
    };

    SDL_GPUGraphicsPipeline *pipeline =
        SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    if (!pipeline) {
        SDL_Log("SDL_CreateGPUGraphicsPipeline failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_ReleaseGPUShader(device, vert);
    SDL_ReleaseGPUShader(device, frag);

    return pipeline;
}

int main() {
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

    SDL_GPUDevice *device =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    if (!device) {
        SDL_Log("SDL_CreateGPUDevice failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("SDL_ClaimWindowForGPUDevice failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_GPUGraphicsPipeline *pipeline = create_pipeline(device, window);
    if (!pipeline) {
        SDL_Log("Failed to create pipeline\n");
        return EXIT_FAILURE;
    }

    const vertex vertices[] = {
        (vertex){{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        (vertex){{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        (vertex){{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        (vertex){{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}}};

    const size_t vertex_count = ARRAY_LENGTH(vertices);

    uint16_t indices[] = {0, 1, 3, 1, 2, 3};

    const size_t index_count = ARRAY_LENGTH(indices);

    SDL_GPUBuffer *vertex_buffer =
        SDL_CreateGPUBuffer(device, &(SDL_GPUBufferCreateInfo){
                                        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                        .size = sizeof(vertex) * vertex_count,
                                    });

    SDL_GPUBuffer *index_buffer =
        SDL_CreateGPUBuffer(device, &(SDL_GPUBufferCreateInfo){
                                        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                                        .size = sizeof(uint16_t) * index_count,
                                    });

    SDL_GPUTransferBuffer *buffer_trans_buffer = SDL_CreateGPUTransferBuffer(
        device, &(SDL_GPUTransferBufferCreateInfo){
                    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                    .size = sizeof(vertex) * vertex_count +
                            sizeof(uint16_t) * index_count,
                });

    void *data = SDL_MapGPUTransferBuffer(device, buffer_trans_buffer, false);
    memcpy(data, vertices, sizeof(vertex) * vertex_count);
    memcpy(data + sizeof(vertex) * vertex_count, indices,
           sizeof(uint16_t) * index_count);
    SDL_UnmapGPUTransferBuffer(device, buffer_trans_buffer);

    SDL_GPUCommandBuffer *upload_cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmdbuf);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){.transfer_buffer = buffer_trans_buffer,
                                         .offset = 0},
        &(SDL_GPUBufferRegion){
            .buffer = vertex_buffer,
            .offset = 0,
            .size = sizeof(vertex) * vertex_count,
        },
        false);

    SDL_UploadToGPUBuffer(copy_pass,
                          &(SDL_GPUTransferBufferLocation){
                              .transfer_buffer = buffer_trans_buffer,
                              .offset = sizeof(vertex) * vertex_count},
                          &(SDL_GPUBufferRegion){
                              .buffer = index_buffer,
                              .offset = 0,
                              .size = sizeof(uint16_t) * index_count,
                          },
                          false);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmdbuf);
    SDL_ReleaseGPUTransferBuffer(device, buffer_trans_buffer);

    bool is_running = true;
    while (is_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                is_running = false;
                break;
            }
        }

        SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(device);
        if (!cmdbuf) {
            SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s\n", SDL_GetError());
            return EXIT_FAILURE;
        }

        SDL_GPUTexture *swapchain_tex;
        if (!SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &swapchain_tex,
                                            NULL, NULL)) {
            SDL_Log("SDL_AcquireGPUSwapchainTexture failed: %s\n",
                    SDL_GetError());
            return EXIT_FAILURE;
        }

        if (!swapchain_tex) {
            SDL_SubmitGPUCommandBuffer(cmdbuf);
            continue;
        }

        SDL_GPUColorTargetInfo color_target_info = {
            .texture = swapchain_tex,
            .clear_color = (SDL_FColor){0.1f, 0.1f, 0.1f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        SDL_GPURenderPass *render_pass =
            SDL_BeginGPURenderPass(cmdbuf, &color_target_info, 1, NULL);

        SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
        SDL_BindGPUVertexBuffers(
            render_pass, 0,
            &(SDL_GPUBufferBinding){.buffer = vertex_buffer, .offset = 0}, 1);
        
        SDL_BindGPUIndexBuffer(render_pass, &(SDL_GPUBufferBinding){.buffer = index_buffer, .offset = 0}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_DrawGPUIndexedPrimitives(render_pass, index_count, 1, 0, 0, 0);

        SDL_EndGPURenderPass(render_pass);

        SDL_SubmitGPUCommandBuffer(cmdbuf);
    }

    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    SDL_ReleaseGPUBuffer(device, vertex_buffer);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}