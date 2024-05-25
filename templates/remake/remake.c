#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>

/*
 * utils.h will be loaded from the framework-root/include directory, it has useful functions
 */
#define UTILS_IMPLEMENTATION
#include <utils.h>

struct remake_state {
	struct loader_shared_state *shared;
	struct rng_state rand_state;
};

void setup(struct loader_shared_state *state) {
	state->remake_state = (struct remake_state *)calloc(1, sizeof(struct remake_state));
	struct remake_state *remake = (struct remake_state *)state->remake_state;
	remake->shared = state;

	xor_init_rng(&remake->rand_state, 0x55780375);
}

void cleanup(struct loader_shared_state *state) {
	struct remake_state *remake = (struct remake_state *)state->remake_state;
	// NOTE(peter): clean up allocations here et.c
	free(state->remake_state);
	state->remake_state = 0;
}

void key_callback(struct remake_state *state, int key) {
}

void audio_callback(struct remake_state *state, int16_t *audio_buffer, size_t frames) {
	memset(audio_buffer, 0, frames*2*sizeof(int16_t));
}

uint32_t mainloop_callback(struct remake_state *state) {

	uint32_t *buffer = state->shared->buffer;
	for(uint32_t i = 0; i < state->shared->buffer_width * state->shared->buffer_height; ++i) {
		buffer[i] = xor_generate_random(&state->rand_state);
	}

	return 0;
}

struct remake_info remake_information = {
	.release_name = "window title",
	.display_name = "display name",
	.author_name = "username",
	.buffer_width = 368,
	.buffer_height = 276,
	.frames_per_second = 50,
	.setup = setup,
	.cleanup = cleanup,
	.key_callback = key_callback,
	.audio_callback = audio_callback,
	.mainloop_callback = mainloop_callback,
};

// NOTE(peter): This is only for windows, as it's too lame to be able to getProcessAddress of the struct, like dlsym can on linux.
EXPORT struct remake_info *get_remake_information() { return &remake_information; }
