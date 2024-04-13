#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Define necessary structures and types
// struct part_state {
//     char window_title[80];
//     char release_name[80];
//     char lib_path[256];
//     uint32_t *buffer;
//     void *user_data;
//     void (*get_information)(struct part_state *state);
//     void (*setup)(struct part_state *state, struct part_state *remakes, int num_remakes);
//     void (*cleanup)(struct part_state *state);
//     void (*audio_callback)(struct part_state *state);
//     void (*key_callback)(struct part_state *state, int key, int scancode, int action, int mods);
//     int (*mainloop_callback)(struct part_state *state);
// };

struct part_state selector;    // Active selector
struct part_state remake;    // Active remake
struct part_state *remakes;    // Array of remakes
HMODULE remake_handle;    // Handle to loaded remake
uint32_t num_remakes;    // Number of remakes

// Function prototypes
// void load_remakes();
// void load_selector();
// void load_remake(uint32_t index);
// void close_remake();

// int main() {
//     load_remakes();
//     load_selector();

//     // Uncomment the following line to load a specific remake
//     // load_remake(index - 1);

//     return 0;
// }

void load_remakes() {
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	char search_path[MAX_PATH];
	DWORD num_files = 0;

	snprintf(search_path, sizeof(search_path), "remakes\\remake_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			num_files++;
		} while (FindNextFile(hFind, &find_data));
		FindClose(hFind);
	}

	// Allocate memory for the remake states
	remakes = (struct part_state *)malloc(num_files * sizeof(struct part_state));
	if (remakes == NULL) {
		printf("Memory allocation failed.\n");
		return;
	}

	// Load the remakes
	snprintf(search_path, sizeof(search_path), "remakes\\remake_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if (hFind != INVALID_HANDLE_VALUE) {
		int index = 0;
		do {
			snprintf(remakes[index].lib_path, sizeof(remakes[index].lib_path), "remakes\\%s", find_data.cFileName);
			HMODULE handle = LoadLibrary(remakes[index].lib_path);
			if (handle != NULL) {
				// Get the pointer to get_information() and call it
				void (*get_info)(struct part_state *) = (void (*)(struct part_state *))GetProcAddress(handle, "get_information");
				if (get_info != NULL) {
					get_info(&remakes[index]);
				}
				FreeLibrary(handle);
			}
			index++;
		} while (FindNextFile(hFind, &find_data));
		FindClose(hFind);
	} else {
		printf("Failed to open remakes directory.\n");
	}
}

void load_selector() {
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	char search_path[MAX_PATH];
	DWORD num_files = 0;
	char *selected_file = NULL;

	snprintf(search_path, sizeof(search_path), "remakes\\selector_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			num_files++;
		} while (FindNextFile(hFind, &find_data));
		FindClose(hFind);
	}

	// Randomly pick one of the files
	if (num_files > 0) {
		srand(time(0));
		int selected_index = rand() % num_files;

		// Find the selected file
		int index = 0;
		hFind = FindFirstFile(search_path, &find_data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (index == selected_index) {
					selected_file = _strdup(find_data.cFileName);
					break;
				}
				index++;
			} while (FindNextFile(hFind, &find_data));
			FindClose(hFind);
		}
	}

	// Load the selected file
	if (selected_file != NULL) {
		char path[256];
		snprintf(path, sizeof(path), "remakes\\%s", selected_file);
		HMODULE handle = LoadLibrary(path);
		if (handle != NULL) {
			// Get pointers to functions in the struct and store them in the global 'selector' struct
			selector.setup = (void (*)(struct part_state *, struct part_state *, uint32_t))GetProcAddress(handle, "setup");
			selector.cleanup = (void (*)(struct part_state *))GetProcAddress(handle, "cleanup");
			selector.audio_callback = (void (*)(struct part_state *, int16_t *, size_t))GetProcAddress(handle, "audio_callback");
			selector.key_callback = (void (*)(struct part_state *, int, int, int, int))GetProcAddress(handle, "key_callback");
			selector.mainloop_callback = (int (*)(struct part_state *))GetProcAddress(handle, "mainloop_callback");

			// Call the setup function
			if (selector.setup != NULL) {
				selector.setup(&selector, NULL, 0);
			}
			free(selected_file);
		}
	}
}

void load_remake(uint32_t index) {
	char search_path[MAX_PATH];
	WIN32_FIND_DATA find_data;
	HANDLE hFind;

	snprintf(search_path, sizeof(search_path), "%s", remakes[index].lib_path);
	HMODULE handle = LoadLibrary(search_path);
	if (handle == NULL) {
		fprintf(stderr, "Error: Unable to load remake.\n");
		exit(EXIT_FAILURE);
	}

	// Get function pointers using GetProcAddress() and store them in the global variable 'remake'
	// remake.get_information = (void (*)(struct part_state *))GetProcAddress(remake_handle, "get_information");
	remake.setup = (void (*)(struct part_state *, struct part_state *, uint32_t))GetProcAddress(handle, "setup");
	remake.cleanup = (void (*)(struct part_state *))GetProcAddress(handle, "cleanup");
	remake.audio_callback = (void (*)(struct part_state *, int16_t *, size_t))GetProcAddress(handle, "audio_callback");
	remake.key_callback = (void (*)(struct part_state *, int, int, int, int))GetProcAddress(handle, "key_callback");
	remake.mainloop_callback = (int (*)(struct part_state *))GetProcAddress(handle, "mainloop_callback");
	if (remake.setup == NULL || remake.cleanup == NULL || remake.audio_callback == NULL || remake.key_callback == NULL || remake.mainloop_callback == NULL) {
		fprintf(stderr, "Error: Unable to get function pointers.\n");
		exit(EXIT_FAILURE);
	}
}

void close_remake() {
	// Close the shared library
	if (remake_handle != NULL) {
		FreeLibrary(remake_handle);
		remake_handle = NULL;
	}
}





