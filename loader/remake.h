#pragma once

/*
 * -=[*]=- remake struct documentation
 *
 * The window-title will be a combination of release_name + author_name + some information from the loader.
 *
 *              lib_path - Internal, do not touch; Name of the shared library to load (the remake);
 *          release_name - Name displayed in selector, maximum 40 characters + null: "Northstar - Megademo II"
 *          display_name - Full name, will be displayed with `loader --list` maximum length 80 characters including null
 *           author_name - Name of the author of the remake.
 *   get_information(..) - Where we fill in the above information, except for lib_path, that is set in the library_loader
 *             setup(..) - Allocate/convert/load whatever data you need for the remake/selector, setup callbacks!
 *           cleanup(..) - Free the above
 *    audio_callback(..) - This will be called when the audio processing stuff needs more audio-samples
 *      key_callback(..) - This is the function that will be called when there are new keypresses to handle, this can be null.
 * mainloop_callback(..) - This will be called 50 times per second, exit by returning non zero
 *
 * change_resolution(..) - This pointer is filled in from the loader, and will let the remake/selector change resolution if they want.
 */

struct loader_state;
struct remake_state;

struct remake_info {
	char *release_name;
	char *display_name;
	char *author_name;
	void (*setup)(struct loader_shared_state *state);
	void (*cleanup)(struct loader_shared_state *state);
	void (*audio_callback)(struct remake_state *state, int16_t *audio_buffer, size_t frames);
	void (*key_callback)(struct remake_state *state, int key, int action);
	void (*mouse_button_callback)(struct remake_state *state, int button, int action);
	uint32_t (*mainloop_callback)(struct remake_state *state);
	void (*change_resolution)(struct loader_state *state, int width, int height);
	struct loader_state *private;
	uint32_t	frames_per_second;
	uint32_t buffer_width;
	uint32_t buffer_height;
};
