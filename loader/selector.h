#pragma once

/*
 * -=[*]=- selector struct documentation
 *
 *          window_title - This is the window title, some information will be added by the loader when displayed.
 *             setup(..) - Allocate/convert/load whatever data you need for the remake/selector, setup callbacks!
 *           cleanup(..) - Free the above
 *    audio_callback(..) - This will be called when the audio processing stuff needs more audio-samples
 *      key_callback(..) - This is the function that will be called when there are new keypresses to handle, this can be null.
 * mainloop_callback(..) - This will be called 50 times per second, exit by returning non zero
 */

struct selector_info {
	char *window_title;
	void (*setup)(struct loader_shared_state *state, struct loader_info *remakes, uint32_t remake_count);
	void (*cleanup)(struct loader_shared_state *state);
	void (*audio_callback)(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames);
	void (*key_callback)(struct loader_shared_state *state, int key);
	void (*pre_selector_run)(struct loader_shared_state *state);
	int32_t (*mainloop_callback)(struct loader_shared_state *state);
	uint32_t	frames_per_second;
	uint32_t buffer_width;
	uint32_t buffer_height;
};

