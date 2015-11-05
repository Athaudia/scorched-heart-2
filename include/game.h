#pragma once

#include "terrain.h"
#include "material.h"

struct game
{
	struct terrain* terrain;
	struct material** materials;
	int material_count;
	int tick;
};

struct game* game__new(struct ag_vec2i size);
void game__destroy(struct game* game);
