#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

#include <vector>
#include <memory>
#include <cmath>

#include "chunk.hpp"

Rectangle get_src_rect(int dx, int dy) {
	int ox = -dx;
	int oy = -dy;

	Rectangle src = {
		(ox * 16) + 16,
		(oy * 16) + 16,
		16,
		16
	};

	return src;
}

int main() {
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "Quadcraft");

	Texture2D stone_texture = LoadTexture("res/textures/texture.png");
	SetTextureFilter(stone_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(stone_texture, TEXTURE_WRAP_CLAMP);

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
			chunk->set_fg_tile(x, y, qc::tile_type::DIRT);
			chunk->set_bg_tile(x, y, qc::tile_type::DIRT);
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
			int x = (int)std::floor(mouse_pos.x / 16.0);
			int y = (int)std::floor(mouse_pos.y / 16.0);

			chunk->set_fg_tile(x, y, qc::tile_type::AIR);
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			int x = (int)std::floor(mouse_pos.x / 16.0);
			int y = (int)std::floor(mouse_pos.y / 16.0);

			chunk->set_fg_tile(x, y, qc::tile_type::DIRT);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE, SKYBLUE);

		BeginMode2D(camera);
		BeginShaderMode(shader);

		Vector2 top_left = GetScreenToWorld2D({0, 0}, camera);
		Vector2 bottom_left = GetScreenToWorld2D(Vector2{ (float)GetScreenWidth(), (float)GetScreenHeight() }, camera);

		int start_x = (int)std::floor(top_left.x / 16.0) - 1;
		int start_y = (int)std::floor(top_left.y / 16.0) - 1;
		int end_x = (int)std::floor(bottom_left.x / 16.0) + 1;
		int end_y = (int)std::floor(bottom_left.y / 16.0) + 1;

		for (int x = start_x; x < end_x; x++) {
			for (int y = start_y; y < end_y; y++) {
				auto fg = chunk->get_fg_tile(x, y);
				auto bg = chunk->get_bg_tile(x, y);

				if (fg == qc::tile_type::AIR) {
					if (bg != qc::tile_type::AIR) {
						DrawTexture(stone_texture, x * 16.0f, y * 16.0f, GRAY);

						bool n =  chunk->get_fg_tile(x + 0, y - 1) != qc::tile_type::AIR;
						bool e =  chunk->get_fg_tile(x + 1, y + 0) != qc::tile_type::AIR;
						bool s =  chunk->get_fg_tile(x + 0, y + 1) != qc::tile_type::AIR;
						bool w =  chunk->get_fg_tile(x - 1, y + 0) != qc::tile_type::AIR;
						bool ne = chunk->get_fg_tile(x + 1, y - 1) != qc::tile_type::AIR;
						bool se = chunk->get_fg_tile(x + 1, y + 1) != qc::tile_type::AIR;
						bool sw = chunk->get_fg_tile(x - 1, y + 1) != qc::tile_type::AIR;
						bool nw = chunk->get_fg_tile(x - 1, y - 1) != qc::tile_type::AIR;

						Color ao_col = { 255, 255, 255, 200 };

						if (n) {
							DrawTextureRec(ao_texture, get_src_rect(0, -1), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (e) {
							DrawTextureRec(ao_texture, get_src_rect(1, 0), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (s) {
							DrawTextureRec(ao_texture, get_src_rect(0, 1), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (w) {
							DrawTextureRec(ao_texture, get_src_rect(-1, 0), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (ne && !n && !e) {
							DrawTextureRec(ao_texture, get_src_rect(1, -1), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (se && !s && !e) {
							DrawTextureRec(ao_texture, get_src_rect(1, 1), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (sw && !s && !w) {
							DrawTextureRec(ao_texture, get_src_rect(-1, 1), { x * 16.0f, y * 16.0f }, ao_col);
						}

						if (nw && !n && !w) {
							DrawTextureRec(ao_texture, get_src_rect(-1, -1), { x * 16.0f, y * 16.0f }, ao_col);
						}
					}
				}

				if (fg != qc::tile_type::AIR) {
					DrawTexture(stone_texture, x * 16.0f, y * 16.0f, WHITE);
				}
			}
		}

		EndShaderMode();
		EndMode2D();
		EndDrawing();
	}

	CloseWindow();
}