#pragma once

#ifdef _WIN32
	#include <intrin.h>
	#include <windows.h>
	static void mks_sleep(double time) { Sleep((DWORD)(time*1000)); }

#elif __linux__
	#include <sys/prctl.h>
	#include <sys/resource.h>
	#include <unistd.h>
	static void mks_sleep(double time) { usleep((int)(time*1000000)); }
#endif

// static const float vertices[] = {
// 	 1.f,-1.f, 0.0f, 1.f, 0.f,
// 	 1.f, 1.f, 0.0f, 1.f, 1.f,
// 	-1.f, 1.f, 0.0f, 0.f, 1.f,
// 	-1.f,-1.f, 0.0f, 0.f, 0.f
// };

static const float vertices[] = {
	 1.f,-1.f, 1.f, 0.f,
	 1.f, 1.f, 1.f, 1.f,
	-1.f, 1.f, 0.f, 1.f,
	-1.f,-1.f, 0.f, 0.f
};
static const uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };

struct viewport { float x, y, w, h; };

enum program_mode {
	LOAD_SELECTOR_STATE,
	SELECTOR_STATE,
	LOAD_REMAKE_STATE,
	REMAKE_STATE,
	UNLOAD_REMAKE_STATE,
};

/*
 * -=[*]=- loader_state struct documentation
 */
struct loader_state {
	struct selector_info *selector;		// Active selector
	struct remake_info *remake;			// Active remake
	struct loader_info *remakes;			// Array of loader_info structs, used by the selector to display names of remakes.
	struct loader_shared_state shared;	//
	struct viewport viewport;				//
	double frame_time;
	uint32_t frames_per_second;
	uint32_t remake_count;					// Number of remakes
	GLuint texture;							//
	enum program_mode mode;					//
	bool toggle_crt_emulation;				// Toggle for the shader CRT emulation.
	bool cursor_locked;						// mouse lock state.
#ifdef _WIN32
	HMODULE remake_handle;					// Handle to loaded remake
#elif defined(__linux__)
	void *remake_handle;						// handle holding the descriptor returned from dlopen
#endif
};

struct debugger_timing {
	double time_start;
	double time_end;
	double time_duration;
	double time_remaining;
};
