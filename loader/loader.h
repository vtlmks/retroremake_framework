#pragma once

// NOTE(peter): DO NOT CHANGE THESE, they are here for convenience!
//              There is nothing that checks these values, they are for optimization only
#define BUFFER_WIDTH (368)
#define BUFFER_HEIGHT (276)
#define FRAMES_PER_SECOND 50.0
#define FRAME_TIME (1.0/FRAMES_PER_SECOND);

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


// TODO(peter): Add more callbacks, mousemove and mousebuttons
// perhaps instead of having callbacks for mousemove and mousebuttons I add a pointer to a datastructure where I publish data like
// delta mouse and other information. For delta mouse you need to save the previous value and subtract from the newly read value
// to get the delta since last read. I might do the same for mousebuttons. The mousebutton bits are set till they are released.
// This will mean that it's possible to miss mouseclicks that are below 20ms long (1/50th frame).

// Instead of using callbacks for mouse movement and mouse buttons, we could consider adding a pointer to a data structure where
// we publish information such as delta mouse movements and other relevant data.
// For calculating delta mouse movements, we'll need to store the previous mouse position and subtract it from the newly read position to get the change since the last read.
// Similarly, we might apply the same approach for tracking mouse button states. The mouse button bits remain set until they are released.
// However, it's worth noting that with this approach, there's a possibility of missing mouse clicks that are shorter than 20 milliseconds (1/50th of a frame).

/*
 * -=[*]=- part_state struct documentation
 *
 *          window_title	// "Northstar - Megademo II - 1987 -- Remake by ViTAL / Mindkiller Systems."
 *          release_name	// Including groupname: "Northstar - Megademo II"
 *              lib_path	// Name of the shared library to load (the remake); This will be set internaly, do not touch
 *                buffer	// The buffer that you will be rendering to ARGB, check buffer size in the defines above
 *             user_data	//
 *   get_information(..)	// Where we fill in the above information, except for lib_path, that is set in the library_loader
 *             setup(..)	// allocate/convert/load whatever data you need for the remake/selector, setup callbacks!
 *           cleanup(..)	// Free the above
 *    audio_callback(..)	// this will be called when the audio processing stuff needs more audio-samples
 *      key_callback(..)	// this is the function that will be called when there are new keypresses to handle, this can be null.
 * mainloop_callback(..)	// this will be called 50 times per second, exit by returning non zero
 */
struct part_state {
	char window_title[80];
	char release_name[80];
	char lib_path[256];
	uint32_t *buffer;
	void *user_data;
	void (*get_information)(struct part_state *state);
	void (*setup)(struct part_state *state, struct part_state *remakes, uint32_t num_remakes);
	void (*cleanup)(struct part_state *state);
	void (*audio_callback)(struct part_state *state);
	void (*key_callback)(struct part_state *state, int key, int scancode, int action, int mods);
	int32_t (*mainloop_callback)(struct part_state *state);
	int32_t mouse_delta_x;
	int32_t mouse_delta_y;
};
