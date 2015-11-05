#include "game.h"

struct game* game__new(struct ag_vec2i size)
{
	struct game* game = (struct game*)malloc(sizeof(struct game));
	game->terrain = terrain__new(size);
	game->material_count = 3;
	game->materials = (struct material**)malloc(sizeof(struct material*)*game->material_count);
	game->materials[0] = material__new();
	material__add_color(game->materials[0], ag_color32( 100,100,250, 0xff));
	game->materials[0]->outline = false;

	game->materials[1] = material__new();
	material__add_color(game->materials[1], ag_color32( 80,  50, 10, 0xff));
	material__add_color(game->materials[1], ag_color32(100,  70, 20, 0xff));
	material__add_color(game->materials[1], ag_color32(120,  90, 40, 0xff));
	material__add_color(game->materials[1], ag_color32(140, 110, 60, 0xff));
	game->materials[1]->kind = MATERIAL_SOLID;

	game->materials[2] = material__new();
	material__add_color(game->materials[2], ag_color32( 0,50,100, 0xff));
	game->materials[2]->kind = MATERIAL_LIQUID;

	game->terrain->materials = game->materials;
	game->tick = 0;
	return game;
}

void game__destroy(struct game* game)
{
	for(int i = 0; i < game->material_count; ++i)
		material__destroy(game->materials[i]);
	free(game->materials);
	terrain__destroy(game->terrain);
	free(game);
}
