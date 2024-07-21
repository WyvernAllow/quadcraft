#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

int main(void) {
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "Quadcraft");

	Texture2D tex = LoadTexture("res/textures/texture.png");
	SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);

	Shader shader = LoadShader(NULL, "res/shaders/pixel_aa.frag");

	Camera2D camera = {
		.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
		.rotation = 0.0f,
		.zoom = 1.0f,
		.target = (Vector2){ 0.0f, 0.0f },
	};

	float target_zoom = 1.0f;
	Vector2 target_pos = (Vector2){ 0.0f, 0.0f };

	// SetTargetFPS(60);
	
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

		camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
		target_pos = Vector2Add(target_pos, Vector2Scale(Vector2Normalize(wish_dir), GetFrameTime() * 64.0));

		camera.target = Vector2Lerp(camera.target, target_pos, 1.0 * GetFrameTime());

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE, SKYBLUE);

		BeginMode2D(camera);

		BeginShaderMode(shader);

		for (size_t x = 0; x < 16; x++) {
			for (size_t y = 0; y < 16; y++) {
				DrawTexture(tex, x * 16, y * 16, WHITE);
			}
		}

		EndShaderMode(shader);

		EndMode2D();

		EndDrawing();
	}

	CloseWindow();
}