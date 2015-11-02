#include <athgame.h>

struct state
{
	int tick;
	struct ag_surface32* test;
};

void* mystate_enter()
{
	struct state* state = (struct state*)malloc(sizeof(struct state));
	state->tick = 100;
	state->test = ag_surface32__new_from_file_with_color_key("test.bmp", agc_magic_pink);
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
	struct state* state = (struct state*) data;
	//state->tick = state->tick + 1;
	ag_surface32__clear(window->surface, ag_color32(state->tick%256, 0, 0, 255));
	ag_surface32__blit_with_color_key_to(window->surface, state->test, ag_vec2i(0,0), agc_transparent);
}


int main()
{
	ag_init();
	struct ag_window* window = ag_window__new(ag_vec2i(320, 240), false);
	ag_window__add_filter(window, AG_FILTER_UP4_NN);
	struct ag_state* state = ag_state__new(window, 60, mystate_enter, mystate_render, mystate_update, 0);
	ag_state__run(state);
	ag_uninit();
	return 0;
}
