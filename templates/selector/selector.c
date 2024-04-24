#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>
#include <selector.h>

#include "../pcg.c"

struct selector {
};

EXPORT void setup(struct loader_shared_state *state, struct loader_info *remakes, uint32_t remake_count) {
	state->selector_userdata = (struct selector *)calloc(1, sizeof(struct selector));
}

EXPORT void cleanup(struct loader_shared_state *state) {
	struct selector *selector = (struct selector *)state->selector_userdata;

	free(state->selector_userdata);
	state->selector_userdata = 0;
}

EXPORT void key_callback(struct loader_shared_state *state, int key) {
	struct selector *selector = (struct selector *)state->selector_userdata;
}

EXPORT void audio_callback(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames) {
	struct selector *selector = (struct selector *)state->selector_userdata;
	memset(audio_buffer, 0, frames*2*sizeof(int16_t));
}

EXPORT int32_t mainloop_callback(struct loader_shared_state *state) {
	struct selector *selector = (struct selector *)state->selector_userdata;

	uint32_t *buffer = state->buffer;
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		buffer[i] = pcg32_random();
	}

	return 0;
}

EXPORT struct selector_info selector_information = {
	.window_title = "window title",
	.setup = setup,
	.cleanup = cleanup,
	.key_callback = key_callback,
	.audio_callback = audio_callback,
	.mainloop_callback = mainloop_callback,
};

// NOTE(peter): This is only for windows, as it's too lame to be able to getProcessAddress of the struct, like dlsym can on linux.
EXPORT struct selector_info *get_selector_information() { return &selector_information; }
