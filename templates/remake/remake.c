#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <loader.h>
#include <remake.h>

struct remake  {
};

EXPORT void setup(struct loader_shared_state *state) {
	state->remake_userdata = (struct remake *)calloc(1, sizeof(struct remake));
}

EXPORT void cleanup(struct loader_shared_state *state) {
	struct remake *remake = (struct remake *)state->remake_userdata;

	free(state->remake_userdata);
	state->remake_userdata = 0;
}

EXPORT void key_callback(struct loader_shared_state *state, int key) {
	struct remake *remake = (struct remake *)state->remake_userdata;
}

EXPORT void audio_callback(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames) {
	struct remake *remake = (struct remake *)state->remake_userdata;
}

EXPORT int32_t mainloop_callback(struct loader_shared_state *state) {
	struct remake *remake = (struct remake *)state->remake_userdata;

	uint32_t *buffer = state->buffer;
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		buffer[i] = rand() * rand();
	}

	return 0;
}

struct remake_info remake_information = {
	.release_name = "window title",
	.display_name = "display name",
	.author_name = "username",
	.setup = setup,
	.cleanup = cleanup,
	.key_callback = key_callback,
	.audio_callback = audio_callback,
	.mainloop_callback = mainloop_callback,
};

// NOTE(peter): This is only for windows, as it's too lame to be able to getProcessAddress of the struct, like dlsym can on linux.
EXPORT struct remake_info *get_remake_information() {
	return &remake_information;
}
