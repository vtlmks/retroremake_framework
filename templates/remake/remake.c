#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>

EXPORT void get_information(struct remake_state *state) {
	strcpy(state->window_title, "This is the window title...");
	strcpy(state->release_name, "Northstar & Fairlight - Megademo III");	// maximum 36 characters, else it will be cut by selectors.
}

EXPORT void setup(struct loader_shared_state *state) {
}

EXPORT void cleanup(struct loader_shared_state *state) {
}

EXPORT void key_callback(struct loader_shared_state *state, int key) {
}

EXPORT void audio_callback(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames) {
}

EXPORT int32_t mainloop_callback(struct loader_shared_state *state) {
	uint32_t *buffer = state->buffer;
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		buffer[i] = rand() * rand();
	}

	return 0;
}

