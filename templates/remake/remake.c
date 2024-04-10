#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <loader.h>

EXPORT void get_information(struct part_state *state) {
	strcpy(state->window_title, "This is the window title...");
	strcpy(state->release_name, "Northstar & Fairlight - Megademo III");	// maximum 36 characters, else it will be cut by selectors.
}

EXPORT void setup(struct part_state *state, void *, void *) {	// the last two arguments we don't care about as a remake.
}

EXPORT void cleanup(struct part_state *state) {
}

EXPORT void key_callback(struct part_state *state, int key, int scancode, int action, int mods) {
}

EXPORT void audio_callback(struct part_state *state) {
}

EXPORT int32_t mainloop_callback(struct part_state *state) {
	uint32_t *buffer = state->buffer;
	static uint32_t arr = 0;
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		buffer[i] = i*46 + arr;
	}
	arr -= 0x400;

	return 0;
}

