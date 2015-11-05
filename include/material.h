#pragma once

#include <athgame.h>
#include <stdbool.h>

enum material_kind {MATERIAL_AIR, MATERIAL_SOLID, MATERIAL_LIQUID};

struct material
{
	struct ag_color32* colors;
	int color_count;
	bool outline;
	enum material_kind kind;
};

struct material* material__new();
void material__destroy(struct material* mat);
void material__add_color(struct material* mat, struct ag_color32 col);
