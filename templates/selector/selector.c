#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <loader.h>
#include <remake.h>
#include <selector.h>

EXPORT void setup(struct selector_state *state, void *, void *) {
	strcpy(state->window_title, "First selector - (c)2024 ViTAL - Mindkiller Systems.");
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
	static uint32_t arr = 0;
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		buffer[i] = i*46 + arr;
	}
	arr += 0x400;

	return 0;
}

