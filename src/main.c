#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

int main(void) {
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(800, 450, "Quadcraft");

	Texture2D tex = LoadTexture("res/textures/texture.png");
	SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);

	Camera2D camera = {
		.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
		.rotation = 0.0f,
		.zoom = 1.0f,
		.target = (Vector2){ 0.0f, 0.0f },
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
			camera.zoom *= 0.9;
		}
		else if (GetMouseWheelMove() > 0.0) {
			camera.zoom /= 0.9;
		}

		camera.target = Vector2Add(camera.target, Vector2Scale(Vector2Normalize(wish_dir), GetFrameTime() * 32.0f));

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(camera);
		
		DrawTexture(tex, 0, 0, WHITE);

		EndMode2D();

		EndDrawing();
	}

	CloseWindow();
}