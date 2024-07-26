#include "chunk.hpp"

namespace qc {
	tile chunk::get_tile(int x, int y) const {
		if (!in_bounds(x, y)) {
			return tile();
		}

		return m_tiles[get_index(x, y)];
	}

	void chunk::set_foreground_tile(int x, int y, tile_type type) {
		if (!in_bounds(x, y)) {
			return;
		}

		m_tiles[get_index(x, y)].foreground = type;
	}

	void chunk::set_background_tile(int x, int y, tile_type type) {
		if (!in_bounds(x, y)) {
			return;
		}

		m_tiles[get_index(x, y)].background = type;
	}

	bool chunk::in_bounds(int x, int y) const {
		return x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT;
	}

	size_t chunk::get_index(int x, int y) const {
		return static_cast<size_t>(x + CHUNK_WIDTH * y);
	}
}