#pragma once
#include <cstddef>
#include <array>

namespace qc {
	enum tile_type {
		AIR,
		DIRT,
		GRASS,
		STONE,
	};

	struct tile {
		tile_type foreground;
		tile_type background;

		tile() 
		: foreground(tile_type::AIR), background(tile_type::AIR) {

		}
	};

	static constexpr int CHUNK_WIDTH = 32;
	static constexpr int CHUNK_HEIGHT = 256;
	static constexpr int CHUNK_AREA = CHUNK_WIDTH * CHUNK_HEIGHT;

	class chunk {
	public:
		tile_type get_fg_tile(int x, int y) const;
		tile_type get_bg_tile(int x, int y) const;

		void set_fg_tile(int x, int y, tile_type tile);
		void set_bg_tile(int x, int y, tile_type tile);

	private:
		tile get_tile(int x, int y) const;
		void set_tile(int x, int y, tile tile);

		bool in_bounds(int x, int y) const;
		size_t get_index(int x, int y) const;

	private:
		std::array<tile, CHUNK_AREA> m_tiles;
	};
}