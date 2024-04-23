#pragma once

#ifdef _WIN32
	#include <intrin.h>
	#include <windows.h>
	#include <timeapi.h>
	static void mks_sleep(double time) { Sleep((DWORD)(time*1000)); }

#elif __linux__
	#include <sys/prctl.h>
	#include <sys/resource.h>
	#include <unistd.h>
	static void mks_sleep(double time) { usleep((int)(time*1000000)); }
#endif

static const char* glsl_version = "#version 140";
static const float vertices[] = {
	 1.f,-1.f, 0.0f, 1.f, 0.f,
	 1.f, 1.f, 0.0f, 1.f, 1.f,
	-1.f, 1.f, 0.0f, 0.f, 1.f,
	-1.f,-1.f, 0.0f, 0.f, 0.f
};
static const uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };

struct viewport { float x, y, w, h; };

enum program_mode {
	SELECTOR_MODE,
	LOAD_REMAKE_MODE,
	REMAKE_MODE,
	UNLOAD_REMAKE_MODE
};

/*
 * -=[*]=- loader_state struct documentation
 *
 */
struct loader_state {
	struct selector_state selector;		// Active selector
	struct remake_state remake;			// Active remake
	struct remake_state *remakes;			// Array of possible remakes.
	uint32_t remake_count;					// Number of remakes
	bool toggle_crt_emulation;				// Toggle for the shader CRT emulation.
	struct loader_shared_state shared;	//
	struct viewport viewport;				//
	enum program_mode mode;					//
	bool cursor_locked;						// mouse lock state.
#ifdef _WIN32
	HMODULE remake_handle;					// Handle to loaded remake
#elif defined(__linux__)
	void *remake_handle;						// handle holding the descriptor returned from dlopen
#endif
};
