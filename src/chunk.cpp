#include "chunk.hpp"

namespace qc {
	tile_type chunk::get_fg_tile(int x, int y) const {
		if (!in_bounds(x, y)) {
			return tile_type::AIR;
		}

		return m_tiles[get_index(x, y)].foreground;
	}

	tile_type chunk::get_bg_tile(int x, int y) const {
		if (!in_bounds(x, y)) {
			return tile_type::AIR;
		}

		return m_tiles[get_index(x, y)].background;
	}

	void chunk::set_fg_tile(int x, int y, tile_type tile) {
		if (!in_bounds(x, y)) {
			return;
		}

		m_tiles[get_index(x, y)].foreground = tile;
	}

	void chunk::set_bg_tile(int x, int y, tile_type tile) {
		if (!in_bounds(x, y)) {
			return;
		}

		m_tiles[get_index(x, y)].background = tile;
	}

	bool chunk::in_bounds(int x, int y) const {
		return x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT;
	}

	size_t chunk::get_index(int x, int y) const {
		return static_cast<size_t>(x + CHUNK_WIDTH * y);
	}
}