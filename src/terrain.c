#include "terrain.h"
#include "material.h"

void terrain__dirty(struct terrain* terrain, struct ag_vec2i pos)
{
	ag_list__push_back(terrain->dirty_pixels, ag_vec2i__new(pos.x, pos.y));
}

struct terrain* terrain__new(struct ag_vec2i size)
{
	struct terrain* terrain = (struct terrain*)malloc(sizeof(struct terrain));
	terrain->size = size;
	terrain->surface = ag_surface32__new(size);
	terrain->data = (uint16_t*)malloc(sizeof(uint16_t) * size.w*size.h);
	terrain->dirty_pixels = ag_list__new();
	int steep = 0;
	int max_steep = 128;
	int steep_div = 16;
	int h = size.h/2*steep_div;
	struct ag_rng* rng = ag_rng__new(AG_RNG_XORSHIFT64, 12);
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
		for(int x = 50; x < 600; ++x)
			if(terrain->data[x+y*size.w] == 0)
				terrain->data[x+y*size.w] = rand()%1==0?0:0;
	return terrain;
}

void terrain__destroy(struct terrain* terrain)
{
	ag_surface32__destroy(terrain->surface);
	free(terrain->data);
	free(terrain->dirty_pixels);
	free(terrain);
}

uint32_t garble(uint32_t in)
{
	//todo: find better way
	return in%997 * in%13 * in%631 * in%349;
}

void terrain__update_surface(struct terrain* terrain)
{
	struct material** materials = terrain->materials;
	//for(int y = 0; y < terrain->size.h; ++y)
	//	for(int x = 0; x < terrain->size.w; ++x)
	while(terrain->dirty_pixels->front)
	{
		//printf("1\n");
		struct ag_vec2i* dirty = ag_list__pop_front(terrain->dirty_pixels);
		//printf("2\n");
		int startx = dirty->x-1, stopx = dirty->x+1, starty = dirty->y-1, stopy = dirty->y+1;
		//printf("3 %d\n", (int)dirty);
		ag_vec2i__destroy(dirty);
		//printf("4\n");
		if(startx < 0) startx = 0;
		if(starty < 0) starty = 0;
		if(stopx >= terrain->size.w) stopx = terrain->size.w-1;
		if(stopy >= terrain->size.h) stopy = terrain->size.h-1;
		//printf("startx: %d, stopx: %d, starty: %d, stopy: %d\n", startx, stopx, starty, stopy);
		for(int x = startx; x <= stopx; ++x)
		for(int y = starty; y <= stopy; ++y)
		{
			//printf(".\n");
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
			//printf("-\n");
		}
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
	terrain__dirty(terrain, pos1);
	terrain__dirty(terrain, pos2);
}

bool terrain__tick_physics(struct terrain* terrain)
{
	struct ag_vec2i pos;
	for(int iteration = 0; iteration < 2; ++iteration)
	for(pos.y = terrain->size.h-1; pos.y >= 0; --pos.y)
		for(pos.x = 0; pos.x < terrain->size.w; ++pos.x)
		{
			if(pos.y+1 < terrain->size.h && terrain__get(terrain, pos)->kind == MATERIAL_LIQUID)
			{
				struct ag_vec2i l, r, new_pos = pos;
				bool go_l = true, go_r = true;
				l = r = ag_vec2i(pos.x,pos.y+1);
				if(terrain__get(terrain, ag_vec2i(pos.x,pos.y-1))->kind != MATERIAL_AIR && terrain__get(terrain, ag_vec2i(pos.x,pos.y+1))->kind != MATERIAL_AIR)
					go_l = go_r = false;

				while(go_l || go_r)
				{
					if(go_l && terrain__get(terrain, l)->kind == MATERIAL_AIR)
					{
						//terrain__swap(terrain, pos, l);
						new_pos = l;
						break;
					}

					if(go_r && terrain__get(terrain, r)->kind == MATERIAL_AIR)
					{
						//terrain__swap(terrain, pos, r);
						new_pos = r;
						break;
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
				if(pos.x != new_pos.x || pos.y != new_pos.y)
				{
					int dist = new_pos.x-pos.x;
					if(dist < 0)
						dist = -dist;
					for(int i = 0; i < dist; ++dist)
						if(new_pos.y+1 < terrain->size.h && terrain__get(terrain, ag_vec2i(new_pos.x, new_pos.y+1))->kind == MATERIAL_AIR)
							++new_pos.y;
						else
							break;
					terrain__swap(terrain, pos, new_pos);
				}
			}
		}
	return true;
}
