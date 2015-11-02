#include <athgame.h>

uint32_t garble(uint32_t in)
{
	//todo: find better way
	return in%997 * in%13 * in%631 * in%349;
}

struct state
{
	int tick;
};

void* mystate_enter()
{
	struct state* state = (struct state*)malloc(sizeof(struct state));
	state->tick = 0;
	return state;
}

void mystate_update(void* data, struct ag_window* window)
{
	struct state* state = (struct state*) data;
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
	
	state->tick++;
}

void mystate_render(void* data, struct ag_window* window)
{
	//struct state* state = (struct state*) data;
	ag_surface32__clear(window->surface, agc_black);
	struct ag_color32 earth_pal[4];
	earth_pal[0] = ag_color32( 80,  50, 10, 0xff);
	earth_pal[1] = ag_color32(100,  70, 20, 0xff);
	earth_pal[2] = ag_color32(120,  90, 40, 0xff);
	earth_pal[3] = ag_color32(140, 110, 60, 0xff);
	for(int y = 0; y < window->surface->size.h; ++y)
		for(int x = 0; x < window->surface->size.w; ++x)
		{
			window->surface->data[x+y*window->surface->size.w] = earth_pal[garble(x+(y<<16))%4];
		}
}


int main()
{
	ag_init();
	struct ag_window* window = ag_window__new(ag_vec2i(640*3, 480*3), false);
	//ag_window__add_filter(window, AG_FILTER_UP2_NN);
	struct ag_state* state = ag_state__new(window, 60, mystate_enter, mystate_render, mystate_update, 0);
	ag_state__run(state);
	ag_uninit();
	return 0;
}
