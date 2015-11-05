#pragma once
#include <athgame.h>


struct terrain
{
	uint16_t* data;
	struct ag_vec2i size;
	struct ag_surface32* surface;
	struct material** materials;
	struct ag_list* dirty_pixels;
};


struct terrain* terrain__new(struct ag_vec2i size);
void terrain__destroy(struct terrain* terrain);

void terrain__update_surface(struct terrain* terrain);
struct material* terrain__get(struct terrain* terrain, struct ag_vec2i pos);
void terrain__swap(struct terrain* terrain, struct ag_vec2i pos1, struct ag_vec2i pos2);
void terrain__dirty(struct terrain* terrain, struct ag_vec2i pos);
bool terrain__tick_physics(struct terrain* terrain);
