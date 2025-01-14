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
 *
 * change_resolution(..) - This pointer is filled in from the loader, and will let the remake/selector change resolution if they want.
 */

struct loader_state;
struct selector_state;

struct selector_info {
	char *window_title;
	void (*setup)(struct loader_shared_state *state, struct loader_info *remakes, uint32_t remake_count);
	void (*cleanup)(struct loader_shared_state *state);
	void (*audio_callback)(struct selector_state *state, int16_t *audio_buffer, size_t frames);
	void (*key_callback)(struct selector_state *state, int key, int action);
	void (*mouse_button_callback)(struct selector_state *state, int button, int action);
	void (*pre_selector_run)(struct selector_state *state);
	uint32_t (*mainloop_callback)(struct selector_state *state);
	void (*change_resolution)(struct loader_state *state, int width, int height);
	uint32_t	frames_per_second;
	uint32_t buffer_width;
	uint32_t buffer_height;
	struct loader_state *private;
};

