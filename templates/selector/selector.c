#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>
#include <selector.h>

/*
 * utils.h has some useful functions, like fast and also more secure random number functions, more will be added over time.
 */

#define UTILS_IMPLEMENTATION
#include "utils.h"

struct selector_state {
	struct loader_shared_state *shared;
	struct rng_state rand_state;
};

void setup(struct loader_shared_state *state, struct loader_info *remakes, uint32_t remake_count) {
	state->selector_state = (struct selector_state *)calloc(1, sizeof(struct selector_state));
	struct selector_state *selector = (struct selector_state *)state->selector_state;
	selector->shared = state;

	xor_init_rng(&selector->rand_state, 0x44780142);
}

void pre_selector_run(struct selector_state *state) {
	(void) state;
}

void cleanup(struct loader_shared_state *state) {
	struct selector_state *selector = (struct selector_state *)state->selector_state;
	(void) selector;

	free(state->selector_state);
	state->selector_state = 0;
}

void key_callback(struct selector_state *state, int key) {
	(void) state;
	(void) key;
}

void audio_callback(struct selector_state *state, int16_t *audio_buffer, size_t frames) {
	(void) state;
	memset(audio_buffer, 0, frames*2*sizeof(int16_t));
}

/*
 *  ESCAPE is used globally to exit everything.
 *
 *  The returncode from the mainloop tells the loader what remake to load,
 */
uint32_t mainloop_callback(struct selector_state *state) {
	uint32_t mask = 0xffffffff;

	if(state->shared->mouse_button_state[REMAKE_MOUSE_BUTTON_LEFT]) {
		mask = 0xff00ff00;
	} else if(state->shared->mouse_button_state[REMAKE_MOUSE_BUTTON_RIGHT]) {
		mask = 0x00ff00ff;
	}

	uint32_t *buffer = state->shared->buffer;
	for(uint32_t i = 0; i < state->shared->buffer_width * state->shared->buffer_height; ++i) {
		buffer[i] = xor_generate_random(&state->rand_state) & mask;
	}

	return 0;
}

struct selector_info selector_information = {
	.window_title = "window title",
	.buffer_width = 368,
	.buffer_height = 276,
	.frames_per_second = 50,
	.setup = setup,
	.cleanup = cleanup,
	.key_callback = key_callback,
	.audio_callback = audio_callback,
	.mainloop_callback = mainloop_callback,
	.pre_selector_run = pre_selector_run,
};

// NOTE(peter): This is only for windows, as it's too lame to be able to getProcessAddress of the struct, like dlsym can on linux.
EXPORT struct selector_info *get_selector_information() { return &selector_information; }
