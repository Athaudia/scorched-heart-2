#include <athgame.h>

uint32_t garble(uint32_t in)
{
	//todo: find better way
	return in%997 * in%13 * in%631 * in%349;
}

struct material
{
	struct ag_color32* colors;
	int color_count;
};

struct material* material__new()
{
	struct material* material = (struct material*)malloc(sizeof(struct material));
	material->color_count = 0;
	material->colors = 0;
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

struct terrain
{
	uint16_t* data;
	struct ag_vec2i size;
	struct ag_surface32* surface;
};

struct terrain* terrain__new(struct ag_vec2i size)
{
	struct terrain* terrain = (struct terrain*)malloc(sizeof(struct terrain));
	terrain->size = size;
	terrain->surface = ag_surface32__new(size);
	terrain->data = (uint16_t*)malloc(sizeof(uint16_t) * size.w*size.h);
	for(int x = 0; x < size.w; ++x)
		for(int y = 0; y < size.h; y++)
			terrain->data[x+y*size.w] = y>(x+100)%800?1:0;
	return terrain;
}

void terrain__destroy(struct terrain* terrain)
{
	ag_surface32__destroy(terrain->surface);
	free(terrain->data);
	free(terrain);
}

void terrain__update_surface(struct terrain* terrain, struct material** materials)
{
	for(int y = 0; y < terrain->size.h; ++y)
		for(int x = 0; x < terrain->size.w; ++x)
		{
			struct material* material = materials[terrain->data[x+y*terrain->size.w]];
			terrain->surface->data[x+y*terrain->size.w] = material->colors[garble(x+(y<<16))%material->color_count];
		}
}

struct game
{
	struct terrain* terrain;
	struct material** materials;
	int material_count;
};

struct game* game__new(struct ag_vec2i size)
{
	struct game* game = (struct game*)malloc(sizeof(struct game));
	game->terrain = terrain__new(size);
	game->material_count = 2;
	game->materials = (struct material**)malloc(sizeof(struct material*)*game->material_count);
	game->materials[0] = material__new();
	material__add_color(game->materials[0], ag_color32( 100,100,250, 0xff));
	game->materials[1] = material__new();
	material__add_color(game->materials[1], ag_color32( 80,  50, 10, 0xff));
	material__add_color(game->materials[1], ag_color32(100,  70, 20, 0xff));
	material__add_color(game->materials[1], ag_color32(120,  90, 40, 0xff));
	material__add_color(game->materials[1], ag_color32(140, 110, 60, 0xff));
	
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

void* mystate_enter()
{
	return game__new(ag_vec2i(640*2, 480*2));
}

void mystate_update(void* data, struct ag_window* window)
{
	struct ag_event* event;
	while((event = ag_event__get()))
	{
		switch(event->type)
		{
		case AG_EVENT_CLOSE:
			ag_state__pop();
			break;
		default:
			break;
		}
	}
}

void mystate_render(void* data, struct ag_window* window)
{
	struct game* game = (struct game*) data;
	ag_surface32__clear(window->surface, agc_black);
	terrain__update_surface(game->terrain, game->materials);
	ag_surface32__blit_to(window->surface, game->terrain->surface, ag_vec2i(0,0));
}


int main()
{
	ag_init();
	struct ag_window* window = ag_window__new(ag_vec2i(640*2, 480*2), false);
	//ag_window__add_filter(window, AG_FILTER_UP2_NN);
	struct ag_state* state = ag_state__new(window, 60, mystate_enter, mystate_render, mystate_update, 0);
	ag_state__run(state);
	ag_uninit();
	return 0;
}
