#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

#include <memory>

#include "chunk.hpp"

int main() {
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "Quadcraft");

	Texture2D tex = LoadTexture("res/textures/texture.png");
	SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

	Shader shader = LoadShader(NULL, "res/shaders/pixel_aa.frag");

	Camera2D camera = {};
	camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	camera.target = { 0.0f, 0.0f };

	float target_zoom = 1.0f;
	Vector2 target_pos = { 0.0f, 0.0f };

	Texture2D north = LoadTexture("res/textures/north.png");
	Texture2D east = LoadTexture("res/textures/east.png");
	Texture2D south = LoadTexture("res/textures/south.png");
	Texture2D west = LoadTexture("res/textures/west.png");

	SetTextureFilter(north, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(east, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(south, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(west, TEXTURE_FILTER_BILINEAR);

	// SetTargetFPS(60);
	
	std::unique_ptr<qc::chunk> chunk = std::make_unique<qc::chunk>();

	for (int x = 0; x < qc::CHUNK_WIDTH; x++) {
		for (int y = 0; y < qc::CHUNK_HEIGHT; y++) {
			chunk->set_fg_tile(x, y, qc::tile_type::DIRT);
			chunk->set_bg_tile(x, y, qc::tile_type::DIRT);
		}
	}

	Rectangle player = {
		0.0f,
		0.0f,
		16.0f,
		32.0f
	};
	
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
			int x = (int)floorf(mouse_pos.x / 16.0);
			int y = (int)floorf(mouse_pos.y / 16.0);

			chunk->set_fg_tile(x, y, qc::tile_type::AIR);
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			int x = (int)floorf(mouse_pos.x / 16.0);
			int y = (int)floorf(mouse_pos.y / 16.0);

			chunk->set_fg_tile(x, y, qc::tile_type::DIRT);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE, SKYBLUE);

		BeginMode2D(camera);
		BeginShaderMode(shader);
		BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

		for (int x = 0; x < qc::CHUNK_WIDTH; x++) {
			for (int y = 0; y < qc::CHUNK_HEIGHT; y++) {
				auto fg = chunk->get_fg_tile(x, y);
				auto bg = chunk->get_bg_tile(x, y);

				if (fg == qc::tile_type::AIR) {
					if (bg != qc::tile_type::AIR) {
						DrawTexture(tex, x * 16.0f, y * 16.0f, GRAY);

						bool n = chunk->get_fg_tile(x + 0, y - 1);
						bool e = chunk->get_fg_tile(x + 1, y + 0);
						bool s = chunk->get_fg_tile(x + 0, y + 1);
						bool w = chunk->get_fg_tile(x - 1, y + 0);

						if (n) {
							DrawTexture(north, x * 16.0f, y * 16.0f, WHITE);
						}

						if (e) {
							DrawTexture(east, x * 16.0f, y * 16.0f, WHITE);
						}

						if (s) {
							DrawTexture(south, x * 16.0f, y * 16.0f, WHITE);
						}

						if (w) {
							DrawTexture(west, x * 16.0f, y * 16.0f, WHITE);
						}
					}
				}
				
				if (fg != qc::tile_type::AIR) {
					DrawTexture(tex, x * 16.0f, y * 16.0f, WHITE);
				}
			}
		}

		EndBlendMode();
		EndShaderMode();

		DrawRectangle(player.x, player.y, player.width, player.height, RED);

		EndMode2D();
		EndDrawing();
	}

	CloseWindow();
}