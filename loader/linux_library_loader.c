#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <time.h>

// Comparison function for qsort
static int compare_release_name(const void *a, const void *b) {
	const struct remake_state *part_a = (const struct remake_state *)a;
	const struct remake_state *part_b = (const struct remake_state *)b;
	return strcmp(part_a->release_name, part_b->release_name);
}

// Function to sort an array of part_state structs based on release_name
static void sort_by_release_name(struct remake_state *remakes, size_t num_remakes) {
	qsort(remakes, num_remakes, sizeof(struct remake_state), compare_release_name);
}

/*
 * Traverses the remakes/ directory and obtains information from every shared library that matches remake_*.so
 */
static void load_remakes(struct loader_state *state) {
	DIR *dir;
	struct dirent *ent;
	dir = opendir("remakes");
	if(dir) {
		// Count the number of files matching the pattern
		while((ent = readdir(dir))) {
			if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
				state->remake_count++;
			}
		}
		closedir(dir);

		// Allocate memory for the remake states
		state->remakes = (struct remake_state *)calloc(state->remake_count, sizeof(struct remake_state));

		// Load the remakes
		int index = 0;
		dir = opendir("remakes");
		if(dir) {
			while((ent = readdir(dir))) {
				if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
					snprintf(state->remakes[index].lib_path, sizeof(state->remakes[index].lib_path), "remakes/%s", ent->d_name);
					void *handle = dlopen(state->remakes[index].lib_path, RTLD_LAZY);
					if(handle) {
						// Get the pointer to get_information() and call it
						void (*get_info)(struct remake_state *) = dlsym(handle, "get_information");
						if(get_info) {
							get_info(&state->remakes[index]);
						}
						dlclose(handle);
					}
					index++;
				}
			}
			closedir(dir);
		} else {
			printf("opendir() error!\n");
		}
	} else {
		printf("opendir() error!\n");
	}
}

/*
 *
 */
static void load_selector(struct loader_state *state) {
	DIR *dir;
	struct dirent *ent;
	int num_files = 0;
	char *selected_file = 0;

	// Count the number of files matching the pattern
	dir = opendir("remakes");
	if(dir) {
		while((ent = readdir(dir))) {
			if(fnmatch("selector_*.so", ent->d_name, 0) == 0) {
				num_files++;
			}
		}
		closedir(dir);
	}

	// Randomly pick one of the files
	if(num_files > 0) {
		srand(time(NULL));
		int selected_index = rand() % num_files;

		// Find the selected file
		int index = 0;
		dir = opendir("remakes");
		if(dir) {
			while((ent = readdir(dir))) {
				if(fnmatch("selector_*.so", ent->d_name, 0) == 0) {
					if(index == selected_index) {
						selected_file = strdup(ent->d_name);
						break;
					}
					index++;
				}
			}
			closedir(dir);
		}
	}

	// Load the selected file
	if(selected_file) {
		char path[256];
		snprintf(path, sizeof(path), "remakes/%s", selected_file);
		void *handle = dlopen(path, RTLD_LAZY);
		if(handle) {
			// Get pointers to functions in the struct and store them in the global 'selector' struct
			state->selector.setup = dlsym(handle, "setup");
			state->selector.cleanup = dlsym(handle, "cleanup");
			state->selector.audio_callback = dlsym(handle, "audio_callback");
			state->selector.key_callback = dlsym(handle, "key_callback");
			state->selector.mainloop_callback = dlsym(handle, "mainloop_callback");

// 			// Call the setup function
			if(state->selector.setup) {
				state->selector.setup(&state->shared, 0, 0);
			}
			free(selected_file);
		}
	}
}

/*
 *
 */
static void load_remake(struct loader_state *state, uint32_t index) {
	// Open the shared library
	state->remake_handle = dlopen(state->remakes[index].lib_path, RTLD_LAZY);
	if(!state->remake_handle) {
		fprintf(stderr, "Error: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	// Get function pointers using dlsym() and store them in the global variable 'remake'
	// remake.get_information = (void (*)(struct part_state *))dlsym(remake_handle, "get_information");
	state->remake.setup = (void (*)(struct loader_shared_state *))dlsym(state->remake_handle, "setup");
	state->remake.cleanup = (void (*)(struct loader_shared_state *))dlsym(state->remake_handle, "cleanup");
	state->remake.audio_callback = (void (*)(struct loader_shared_state *, int16_t *, size_t))dlsym(state->remake_handle, "audio_callback");
	state->remake.key_callback = (void (*)(struct loader_shared_state *, int))dlsym(state->remake_handle, "key_callback");
	state->remake.mainloop_callback = (int (*)(struct loader_shared_state *))dlsym(state->remake_handle, "mainloop_callback");
	if(dlerror() != NULL) {
		fprintf(stderr, "Error: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
}

/*
 *
 */
static void close_remake(struct loader_state *state) {
	// Close the shared library
	if(state->remake_handle) {
		dlclose(state->remake_handle);
		state->remake_handle = NULL;
	}
}

/*
 *
 */
// int main() {
// 	load_remakes();
// 	load_selector();

// 	// the selector returns a value that is positive but not zero, so we need -1 to get the correct index.
// //	load_remake(index-1);

// 	return 0;
// }
