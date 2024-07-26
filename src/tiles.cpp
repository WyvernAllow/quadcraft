#include "tiles.hpp"
#include <unordered_map>

namespace qc {
	const std::unordered_map<tile_type, tile_properties> tile_table = {
		{tile_type::AIR, tile_properties {
			.is_transparent = true,
			.atlas_x = 0,
			.atlas_y = 0
		}},

		{tile_type::DIRT, tile_properties {
			.is_transparent = false,
			.atlas_x = 0,
			.atlas_y = 0
		}},

		{tile_type::GRASS, tile_properties {
			.is_transparent = false,
			.atlas_x = 1,
			.atlas_y = 0
		}},

		{tile_type::STONE, tile_properties {
			.is_transparent = false,
			.atlas_x = 2,
			.atlas_y = 0
		}},
	};

	const tile_properties& get_tile_properties(tile_type type) {
		return tile_table.at(type);
	}
}