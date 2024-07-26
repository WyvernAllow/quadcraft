#pragma once
#include <cstddef>
#include <array>

#include "tiles.hpp"

namespace qc {
	static constexpr int CHUNK_WIDTH = 32;
	static constexpr int CHUNK_HEIGHT = 256;
	static constexpr int CHUNK_AREA = CHUNK_WIDTH * CHUNK_HEIGHT;

	class chunk {
	public:
		void set_foreground_tile(int x, int y, tile_type type);
		void set_background_tile(int x, int y, tile_type type);

		tile get_tile(int x, int y) const;

	private:
		bool in_bounds(int x, int y) const;
		size_t get_index(int x, int y) const;

	private:
		std::array<tile, CHUNK_AREA> m_tiles;
	};
}