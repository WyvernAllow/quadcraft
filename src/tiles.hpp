#pragma once

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
			: foreground(tile_type::AIR), background(tile_type::AIR) {}
	};

	struct tile_properties {
		bool is_transparent;
		int atlas_x;
		int atlas_y;
	};

	const tile_properties& get_tile_properties(tile_type type);
}