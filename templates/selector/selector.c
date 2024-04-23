#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>
#include <selector.h>

EXPORT void setup(struct selector_state *state, struct remake_state *remakes, uint32_t remake_count) {
	strcpy(state->window_title, "First selector - (c)2024 ViTAL - Mindkiller Systems.");

	printf("mo:\n");

	for(uint32_t i = 0; i < remake_count; ++i) {
		printf("%s\n", remakes[i].release_name);
	}
}

EXPORT void cleanup(struct loader_shared_state *state) {
}

EXPORT void key_callback(struct loader_shared_state *state, int key) {
}

EXPORT void audio_callback(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames) {
	memset(audio_buffer, 0, frames*2*sizeof(int16_t));
}

EXPORT int32_t mainloop_callback(struct loader_shared_state *state) {
	uint32_t *buffer = state->buffer;
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		buffer[i] = rand() * rand();
	}

	return 0;
}

