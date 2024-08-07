#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

#include <vector>
#include <memory>
#include <cmath>

#include "chunk.hpp"

#include <spdlog/spdlog.h>

#include <cstdarg>

// Cell size in pixels
static constexpr int CELL_SIZE = 16;

Rectangle get_src_rect(int dx, int dy) {
	int ox = -dx;
	int oy = -dy;

	Rectangle src = {
		(ox * CELL_SIZE) + CELL_SIZE,
		(oy * CELL_SIZE) + CELL_SIZE,
		CELL_SIZE,
		CELL_SIZE
	};

	return src;
}

static void raylib_spdlog_callback(int msgType, const char *text, va_list args) {
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), text, args);

	switch (msgType)
	{
	case LOG_INFO:
		spdlog::info("{}", buffer);
		break;
	case LOG_ERROR:
		spdlog::error("{}", buffer);
		break;
	case LOG_WARNING:
		spdlog::warn("{}", buffer);
		break;
	case LOG_DEBUG:
		spdlog::debug("{}", buffer);
		break;
	default:
		spdlog::info("{}", buffer);
		break;
	}
}

int main() {
	SetTraceLogCallback(raylib_spdlog_callback);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(800, 450, "Quadcraft");

	Texture2D atlas = LoadTexture("res/textures/atlas.png");
	SetTextureFilter(atlas, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(atlas, TEXTURE_WRAP_CLAMP);

	Texture2D ao_texture = LoadTexture("res/textures/ao.png");
	SetTextureFilter(ao_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(ao_texture, TEXTURE_WRAP_CLAMP);

	Shader shader = LoadShader(NULL, "res/shaders/pixel_aa.frag");

	Camera2D camera = {};
	camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	camera.target = { 0.0f, 0.0f };

	float target_zoom = 1.0f;
	Vector2 target_pos = { 0.0f, 0.0f };
	
	std::unique_ptr<qc::chunk> chunk = std::make_unique<qc::chunk>();

	for (int x = 0; x < qc::CHUNK_WIDTH; x++) {
		for (int y = 0; y < qc::CHUNK_HEIGHT; y++) {
			chunk->set_foreground_tile(x, y, qc::tile_type::AIR);
			chunk->set_background_tile(x, y, qc::tile_type::DIRT);
		}
	}

	while (!WindowShouldClose()) {
		Vector2 wish_dir = {0.0f, 0.0f};

		if (IsKeyDown(KEY_W)) {
			wish_dir.y--;
		}

		if (IsKeyDown(KEY_S)) {
			wish_dir.y++;
		}

		if (IsKeyDown(KEY_D)) {
			wish_dir.x++;
		}

		if (IsKeyDown(KEY_A)) {
			wish_dir.x--;
		}

		if (GetMouseWheelMove() < 0.0) {
			target_zoom *= 0.9;
		}
		else if (GetMouseWheelMove() > 0.0) {
			target_zoom /= 0.9;
		}

		camera.zoom = Lerp(camera.zoom, target_zoom, 2 * GetFrameTime());

		camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
		target_pos = Vector2Add(target_pos, Vector2Scale(Vector2Normalize(wish_dir), GetFrameTime() * 64.0));

		camera.target = Vector2Lerp(camera.target, target_pos, 1.0 * GetFrameTime());

		Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			int x = (int)std::floor(mouse_pos.x / CELL_SIZE);
			int y = (int)std::floor(mouse_pos.y / CELL_SIZE);

			if (IsKeyDown(KEY_LEFT_SHIFT)) {
				chunk->set_background_tile(x, y, qc::tile_type::STONE);
			}
			else {
				chunk->set_foreground_tile(x, y, qc::tile_type::STONE);
			}
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			int x = (int)std::floor(mouse_pos.x / CELL_SIZE);
			int y = (int)std::floor(mouse_pos.y / CELL_SIZE);

			if (IsKeyDown(KEY_LEFT_SHIFT)) {
				chunk->set_background_tile(x, y, qc::tile_type::AIR);
			}
			else {
				chunk->set_foreground_tile(x, y, qc::tile_type::AIR);
			}
		}

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE, SKYBLUE);

		BeginMode2D(camera);
		BeginShaderMode(shader);

		Vector2 top_left = GetScreenToWorld2D({0, 0}, camera);
		Vector2 bottom_left = GetScreenToWorld2D(Vector2{ (float)GetScreenWidth(), (float)GetScreenHeight() }, camera);

		int start_x = (int)std::floor(top_left.x / CELL_SIZE) - 1;
		int start_y = (int)std::floor(top_left.y / CELL_SIZE) - 1;
		int end_x = (int)std::floor(bottom_left.x / CELL_SIZE) + 1;
		int end_y = (int)std::floor(bottom_left.y / CELL_SIZE) + 1;

		for (int x = start_x; x < end_x; x++) {
			for (int y = start_y; y < end_y; y++) {
				qc::tile tile = chunk->get_tile(x, y);

				const auto& fg_props = qc::get_tile_properties(tile.foreground);
				const auto& bg_props = qc::get_tile_properties(tile.background);

				Vector2 tile_world_pos = { x * CELL_SIZE, y * CELL_SIZE };

				if (fg_props.is_transparent) {
					if (tile.background != qc::tile_type::AIR) {
						DrawTextureRec(atlas, { (float)bg_props.atlas_x * 16, (float)bg_props.atlas_y * 16, 16, 16 }, tile_world_pos, GRAY);

						bool n =  chunk->get_tile(x + 0, y - 1).foreground != qc::tile_type::AIR;
						bool e =  chunk->get_tile(x + 1, y + 0).foreground != qc::tile_type::AIR;
						bool s =  chunk->get_tile(x + 0, y + 1).foreground != qc::tile_type::AIR;
						bool w =  chunk->get_tile(x - 1, y + 0).foreground != qc::tile_type::AIR;
						bool ne = chunk->get_tile(x + 1, y - 1).foreground != qc::tile_type::AIR;
						bool se = chunk->get_tile(x + 1, y + 1).foreground != qc::tile_type::AIR;
						bool sw = chunk->get_tile(x - 1, y + 1).foreground != qc::tile_type::AIR;
						bool nw = chunk->get_tile(x - 1, y - 1).foreground != qc::tile_type::AIR;

						Color ao_col = { 255, 255, 255, 200 };

						if (n) {
							DrawTextureRec(ao_texture, get_src_rect(0, -1), tile_world_pos, ao_col);
						}

						if (e) {
							DrawTextureRec(ao_texture, get_src_rect(1, 0), tile_world_pos, ao_col);
						}

						if (s) {
							DrawTextureRec(ao_texture, get_src_rect(0, 1), tile_world_pos, ao_col);
						}

						if (w) {
							DrawTextureRec(ao_texture, get_src_rect(-1, 0), tile_world_pos, ao_col);
						}

						if (ne && !n && !e) {
							DrawTextureRec(ao_texture, get_src_rect(1, -1), tile_world_pos, ao_col);
						}

						if (se && !s && !e) {
							DrawTextureRec(ao_texture, get_src_rect(1, 1), tile_world_pos, ao_col);
						}

						if (sw && !s && !w) {
							DrawTextureRec(ao_texture, get_src_rect(-1, 1), tile_world_pos, ao_col);
						}

						if (nw && !n && !w) {
							DrawTextureRec(ao_texture, get_src_rect(-1, -1), tile_world_pos, ao_col);
						}
					}
				}

				if (tile.foreground != qc::tile_type::AIR) {
					DrawTextureRec(atlas, { (float)fg_props.atlas_x * 16, (float)fg_props.atlas_y * 16, 16, 16 }, tile_world_pos, WHITE);
				}
			}
		}

		EndShaderMode();
		EndMode2D();
		EndDrawing();
	}

	CloseWindow();
}