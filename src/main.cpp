#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

#include "chunk.hpp"

int main() {
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "Quadcraft");

	Texture2D tex = LoadTexture("res/textures/texture.png");
	SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);

	Shader shader = LoadShader(NULL, "res/shaders/pixel_aa.frag");

	Camera2D camera = {};
	camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	camera.target = { 0.0f, 0.0f };

	float target_zoom = 1.0f;
	Vector2 target_pos = { 0.0f, 0.0f };

	// SetTargetFPS(60);
	
	qc::chunk chunk;

	for (int x = 0; x < qc::CHUNK_WIDTH; x++) {
		for (int y = 0; y < qc::CHUNK_HEIGHT; y++) {
			chunk.set_fg_tile(x, y, qc::tile_type::DIRT);
			chunk.set_bg_tile(x, y, qc::tile_type::DIRT);
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
			int x = (int)floorf(mouse_pos.x / 16.0);
			int y = (int)floorf(mouse_pos.y / 16.0);

			chunk.set_fg_tile(x, y, qc::tile_type::AIR);
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			int x = (int)floorf(mouse_pos.x / 16.0);
			int y = (int)floorf(mouse_pos.y / 16.0);

			chunk.set_fg_tile(x, y, qc::tile_type::DIRT);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE, SKYBLUE);

		BeginMode2D(camera);

		BeginShaderMode(shader);

		SetShapesTexture(tex, { 0, 0, 16, 16 });

		for (int x = 0; x < qc::CHUNK_WIDTH; x++) {
			for (int y = 0; y < qc::CHUNK_HEIGHT; y++) {
				auto fg = chunk.get_fg_tile(x, y);
				auto bg = chunk.get_bg_tile(x, y);

				if (fg == qc::tile_type::AIR) {
					if (bg != qc::tile_type::AIR) {
						bool n =  chunk.get_fg_tile(x + 0, y - 1) != qc::tile_type::AIR;
						bool ne = chunk.get_fg_tile(x + 1, y - 1) != qc::tile_type::AIR;
						bool e =  chunk.get_fg_tile(x + 1, y + 0) != qc::tile_type::AIR;
						bool se = chunk.get_fg_tile(x + 1, y + 1) != qc::tile_type::AIR;
						bool s =  chunk.get_fg_tile(x + 0, y + 1) != qc::tile_type::AIR;
						bool sw = chunk.get_fg_tile(x - 1, y + 1) != qc::tile_type::AIR;
						bool w =  chunk.get_fg_tile(x - 1, y + 0) != qc::tile_type::AIR;
						bool nw = chunk.get_fg_tile(x - 1, y - 1) != qc::tile_type::AIR;

						bool tl = (w || nw || n);
						bool tr = (n || ne || e);
						bool br = (e || se || s);
						bool bl = (w || sw || s);

						Color col1 = tl ? Color{25, 25, 40, 255} : GRAY;
						Color col2 = bl ? Color{25, 25, 40, 255} : GRAY;
						Color col3 = br ? Color{25, 25, 40, 255} : GRAY;
						Color col4 = tr ? Color{25, 25, 40, 255} : GRAY;

						// tl, bl, br, tr
						DrawRectangleGradientEx(Rectangle{ x * 16.0f, y * 16.0f, 16, 16 }, col1, col2, col3, col4);
					}
				}
				
				if (fg != qc::tile_type::AIR) {
					DrawRectangle(x * 16.0f, y * 16.0f, 16, 16, WHITE);
				}
			}
		}

		SetShapesTexture(Texture2D{ 1, 1, 1, 1, 7 }, Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });

		EndShaderMode();

		EndMode2D();

		EndDrawing();
	}

	CloseWindow();
}