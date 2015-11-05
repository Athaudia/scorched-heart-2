#include "material.h"

struct material* material__new()
{
	struct material* material = (struct material*)malloc(sizeof(struct material));
	material->color_count = 0;
	material->colors = 0;
	material->outline = true;
	material->kind = MATERIAL_AIR;
	return material;
}

void material__destroy(struct material* mat)
{
	free(mat->colors);
	free(mat);
}

void material__add_color(struct material* mat, struct ag_color32 col)
{
	mat->colors = realloc(mat->colors, sizeof(ag_color32)*(++mat->color_count));
	mat->colors[mat->color_count-1] = col;
}
