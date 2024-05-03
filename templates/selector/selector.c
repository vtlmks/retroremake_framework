#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>
#include <selector.h>

struct selector {
};

void setup(struct loader_shared_state *state, struct loader_info *remakes, uint32_t remake_count) {
	state->selector_userdata = (struct selector *)calloc(1, sizeof(struct selector));
}

void pre_selector_run(struct loader_shared_state *state) {
	struct selector *selector = (struct selector *)state->selector_userdata;
}

void cleanup(struct loader_shared_state *state) {
	struct selector *selector = (struct selector *)state->selector_userdata;

	free(state->selector_userdata);
	state->selector_userdata = 0;
}

void key_callback(struct loader_shared_state *state, int key) {
	struct selector *selector = (struct selector *)state->selector_userdata;
}

void audio_callback(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames) {
	struct selector *selector = (struct selector *)state->selector_userdata;
	memset(audio_buffer, 0, frames*2*sizeof(int16_t));
}

/*
 *  ESCAPE is used globally to exit everything.
 *
 *  The returncode from the mainloop tells the loader what remake to load,
 */
int32_t mainloop_callback(struct loader_shared_state *state) {
	struct selector *selector = (struct selector *)state->selector_userdata;

	uint32_t mask = 0xffffffff;

	if(state->mouse_button_state[REMAKE_MOUSE_BUTTON_LEFT]) {
		mask = 0xff00ff00;
	} else if(state->mouse_button_state[REMAKE_MOUSE_BUTTON_RIGHT]) {
		mask = 0x00ff00ff;
	}

	uint32_t *buffer = state->buffer;
	// for(uint32_t i = 0; i < state->buffer_width * state->buffer_height; ++i) {
	// 	buffer[i] = pcg32_random() & mask;
	// }

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
