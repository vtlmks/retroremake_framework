#pragma once

/*
 * -=[*]=- selector struct documentation
 *
 *          window_title	// "Northstar - Megademo II - 1987 -- Remake by ViTAL / Mindkiller Systems."
 *   get_information(..)	// Where we fill in the above information, except for lib_path, that is set in the library_loader
 *             setup(..)	// allocate/convert/load whatever data you need for the remake/selector, setup callbacks!
 *           cleanup(..)	// Free the above
 *    audio_callback(..)	// this will be called when the audio processing stuff needs more audio-samples
 *      key_callback(..)	// this is the function that will be called when there are new keypresses to handle, this can be null.
 * mainloop_callback(..)	// this will be called 50 times per second, exit by returning non zero
 */

struct selector_state {
	char window_title[80];
	void (*setup)(struct selector_state *state, struct remake_state *remakes, uint32_t remake_count);
	void (*cleanup)(struct loader_shared_state *state);
	void (*audio_callback)(struct loader_shared_state *state, int16_t *audio_buffer, size_t frames);
	void (*key_callback)(struct loader_shared_state *state, int key);
	int32_t (*mainloop_callback)(struct loader_shared_state *state);
};

