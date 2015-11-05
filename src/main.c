#include <athgame.h>
#include <stdio.h>
#include "game.h"
#include "material.h"
#include "terrain.h"



void* mystate_enter()
{
	return game__new(ag_vec2i(1920/2, 1080/2));
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
	int rain_amount = 0;
	if(game->tick < 60*10)
		rain_amount = game->tick;
	else if(game->tick < 60*20)
		rain_amount = 60*10-(game->tick-60*10);
	for(int i = 0; i < rain_amount/10; ++i)
		game->terrain->data[rand()%game->terrain->size.w+game->terrain->size.w] = 2;
	if(game->tick == 0)
	{
		for(int y = 1; y < game->terrain->size.h-1; y+=3)
			for(int x = 1; x < game->terrain->size.w-1; x+=3)
				terrain__dirty(game->terrain, ag_vec2i(x,y));
	
	}
	terrain__tick_physics(game->terrain);
	++game->tick;
}

void mystate_render(void* data, struct ag_window* window)
{
	struct game* game = (struct game*) data;
	ag_surface32__clear(window->surface, agc_black);
	terrain__update_surface(game->terrain);
	//ag_surface32__blit_to(window->surface, game->terrain->surface, ag_vec2i(0,0));
	ag_surface32__blit_partial_to(window->surface, game->terrain->surface, ag_vec2i(0,0), ag_vec2i(0,0), window->surface->size);
}


int main()
{
	ag_init();
	struct ag_window* window = ag_window__new(ag_vec2i(640, 480), false);
	ag_window__add_filter(window, AG_FILTER_UP2_SCALE2X);
	struct ag_state* state = ag_state__new(window, 60, mystate_enter, mystate_render, mystate_update, 0);
	ag_state__run(state);
	ag_uninit();
	return 0;
}
