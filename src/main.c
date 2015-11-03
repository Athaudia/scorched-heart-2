#include <athgame.h>
#include <stdio.h>

uint32_t garble(uint32_t in)
{
	//todo: find better way
	return in%997 * in%13 * in%631 * in%349;
}

enum material_kind {MATERIAL_AIR, MATERIAL_SOLID, MATERIAL_LIQUID};

struct material
{
	struct ag_color32* colors;
	int color_count;
	bool outline;
	enum material_kind kind;
};

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

struct terrain
{
	uint16_t* data;
	struct ag_vec2i size;
	struct ag_surface32* surface;
	struct material** materials;
};

struct terrain* terrain__new(struct ag_vec2i size)
{
	struct terrain* terrain = (struct terrain*)malloc(sizeof(struct terrain));
	terrain->size = size;
	terrain->surface = ag_surface32__new(size);
	terrain->data = (uint16_t*)malloc(sizeof(uint16_t) * size.w*size.h);
	int steep = 0;
	int max_steep = 128;
	int steep_div = 16;
	int h = size.h/2*steep_div;
	struct ag_rng* rng = ag_rng__new(AG_RNG_XORSHIFT64, 5);
	for(int x = 0; x < size.w; ++x)
	{
		steep += (ag_rng__next_u32(rng)%3-1)*8;
		if(steep > max_steep)
			steep = max_steep;
		else if(steep < -max_steep)
			steep = -max_steep;
		h += steep;
		if(h/steep_div < size.h/4)
		{
			//h = size.h/4*steep_div;
			//steep+=steep_div;
			if(steep < 0)
				steep *= 0.8;
		}
		else if(h/steep_div > size.h/4*3)
		{
			//h = size.h/4*3*steep_div;
			//steep-=steep_div;
			if(steep > 0)
				steep *= 0.8;
		}
		
		for(int y = 0; y < size.h; y++)
			terrain->data[x+y*size.w] = y>(size.h-h/steep_div)?1:0;
	}
	for(int y = 50; y < 200; ++y)
		for(int x = 50; x < 200; ++x)
			if(terrain->data[x+y*size.w] == 0)
				terrain->data[x+y*size.w] = 2;
	return terrain;
}

void terrain__destroy(struct terrain* terrain)
{
	ag_surface32__destroy(terrain->surface);
	free(terrain->data);
	free(terrain);
}

void terrain__update_surface(struct terrain* terrain)
{
	struct material** materials = terrain->materials;
	for(int y = 0; y < terrain->size.h; ++y)
		for(int x = 0; x < terrain->size.w; ++x)
		{
			struct material* material = materials[terrain->data[x+y*terrain->size.w]];
			struct material *l, *u, *d, *r;
			l = u = d = r = material;
			if(x > 0)
				l = materials[terrain->data[x-1+y*terrain->size.w]];
			if(y > 0)
				u = materials[terrain->data[x+(y-1)*terrain->size.w]];
			if(x < terrain->size.w-1)
				r = materials[terrain->data[x+1+y*terrain->size.w]];
			if(y < terrain->size.h-1)
				d = materials[terrain->data[x+(y+1)*terrain->size.w]];

			if(!material->outline && (l->outline || u->outline || r->outline || d->outline))
				terrain->surface->data[x+y*terrain->size.w] = agc_black;
			else
				terrain->surface->data[x+y*terrain->size.w] = material->colors[garble(x+(y<<16))%material->color_count];
		}
}

struct material* terrain__get(struct terrain* terrain, struct ag_vec2i pos)
{
	if(pos.x < 0 || pos.y < 0 || pos.x >= terrain->size.w || pos.y >= terrain->size.h)
		return 0;
	else
		return terrain->materials[terrain->data[pos.x+pos.y*terrain->size.w]];
}

void terrain__swap(struct terrain* terrain, struct ag_vec2i pos1, struct ag_vec2i pos2)
{
	uint16_t m = terrain->data[pos1.x+pos1.y*terrain->size.w];
	terrain->data[pos1.x+pos1.y*terrain->size.w] = terrain->data[pos2.x+pos2.y*terrain->size.w];
	terrain->data[pos2.x+pos2.y*terrain->size.w] = m;
}

bool terrain__tick_physics(struct terrain* terrain)
{
	struct ag_vec2i pos;
	//for(int iteration = 0; iteration < 4; ++iteration)
	for(pos.y = terrain->size.h-1; pos.y >= 0; --pos.y)
		for(pos.x = 0; pos.x < terrain->size.w; ++pos.x)
		{
			if(pos.y+1 < terrain->size.h && terrain__get(terrain, pos)->kind == MATERIAL_LIQUID)
			{
				struct ag_vec2i l, r;
				bool go_l = true, go_r = true;
				l = r = ag_vec2i(pos.x,pos.y+1);
				while(go_l || go_r)
				{
					if(go_l && terrain__get(terrain, l)->kind == MATERIAL_AIR)
					{
						terrain__swap(terrain, pos, l);
						goto done;
					}

					if(go_r && terrain__get(terrain, r)->kind == MATERIAL_AIR)
					{
						terrain__swap(terrain, pos, r);
						goto done;
					}

					if(go_l)
					{
						--l.x;
						if(l.x < 0)
							go_l = false;
						else
						{
							enum material_kind kind = terrain__get(terrain, ag_vec2i(l.x, l.y-1))->kind;
							if(!(kind == MATERIAL_AIR || kind == MATERIAL_LIQUID))
								go_l = false;
						}
					}

					if(go_r)
					{
						++r.x;
						if(r.x >= terrain->size.w)
							go_r = false;
						else
						{
							enum material_kind kind = terrain__get(terrain, ag_vec2i(r.x, r.y-1))->kind;
							if(!(kind == MATERIAL_AIR || kind == MATERIAL_LIQUID))
								go_r = false;
						}
					}
				}
				goto done;
			}
		done:
			;
		}
	return true;
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
	return game__new(ag_vec2i(640, 480));
}

void mystate_update(void* data, struct ag_window* window)
{
	struct game* game = (struct game*) data;
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
	terrain__tick_physics(game->terrain);
}

void mystate_render(void* data, struct ag_window* window)
{
	struct game* game = (struct game*) data;
	ag_surface32__clear(window->surface, agc_black);
	terrain__update_surface(game->terrain);
	ag_surface32__blit_to(window->surface, game->terrain->surface, ag_vec2i(0,0));
}


int main()
{
	ag_init();
	struct ag_window* window = ag_window__new(ag_vec2i(640, 480), false);
	ag_window__add_filter(window, AG_FILTER_UP2_NN);
	struct ag_state* state = ag_state__new(window, 60, mystate_enter, mystate_render, mystate_update, 0);
	ag_state__run(state);
	ag_uninit();
	return 0;
}
