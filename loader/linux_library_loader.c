
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <time.h>

// struct part_state {
// 	char window_title[80];
// 	char release_name[80];
// 	char lib_path[256];
// 	uint32_t *buffer;
// 	void *user_data;
// 	void (*get_information)(struct part_state *state);
// 	void (*setup)(struct part_state *state, struct part_state *remakes, int num_remakes);
// 	void (*cleanup)(struct part_state *state);
// 	void (*audio_callback)(struct part_state *state);
// 	void (*key_callback)(struct part_state *state, int key, int scancode, int action, int mods);
// 	int (*mainloop_callback)(struct part_state *state);
// };

struct part_state selector;		// Active selector
struct part_state remake;			// Active remake
struct part_state *remakes;		// Array of remakes
void *remake_handle;					// handle holding the descriptor returned from dlopen
uint32_t num_remakes;				// Number of remakes


// Comparison function for qsort
int compare_release_name(const void *a, const void *b) {
	const struct part_state *part_a = (const struct part_state *)a;
	const struct part_state *part_b = (const struct part_state *)b;
	return strcmp(part_a->release_name, part_b->release_name);
}

// Function to sort an array of part_state structs based on release_name
void sort_by_release_name(struct part_state *remakes, size_t num_remakes) {
	qsort(remakes, num_remakes, sizeof(struct part_state), compare_release_name);
}

/*
 * Traverses the remakes/ directory and obtains information from every shared library that matches remake_*.so
 */
void load_remakes() {
	DIR *dir;
	struct dirent *ent;
	dir = opendir("remakes");
	if(dir) {
		// Count the number of files matching the pattern
		while((ent = readdir(dir))) {
			if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
				num_remakes++;
			}
		}
		closedir(dir);

		// Allocate memory for the remake states
		remakes = (struct part_state *)calloc(num_remakes, sizeof(struct part_state));

		// Load the remakes
		int index = 0;
		dir = opendir("remakes");
		if(dir) {
			while((ent = readdir(dir))) {
				if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
					snprintf(remakes[index].lib_path, sizeof(remakes[index].lib_path), "remakes/%s", ent->d_name);
					void *handle = dlopen(remakes[index].lib_path, RTLD_LAZY);
					if(handle) {
						// Get the pointer to get_information() and call it
						void (*get_info)(struct part_state *) = dlsym(handle, "get_information");
						if(get_info) {
							get_info(&remakes[index]);
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
void load_selector() {
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
			selector.setup = dlsym(handle, "setup");
			selector.cleanup = dlsym(handle, "cleanup");
			selector.audio_callback = dlsym(handle, "audio_callback");
			selector.key_callback = dlsym(handle, "key_callback");
			selector.mainloop_callback = dlsym(handle, "mainloop_callback");

// 			// Call the setup function
			if(selector.setup) {
				selector.setup(&selector, 0, 0);
			}
			free(selected_file);
		}
	}
}

/*
 *
 */
void load_remake(uint32_t index) {
	// Open the shared library
	remake_handle = dlopen(remakes[index].lib_path, RTLD_LAZY);
	if(!remake_handle) {
		fprintf(stderr, "Error: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	// Get function pointers using dlsym() and store them in the global variable 'remake'
	// remake.get_information = (void (*)(struct part_state *))dlsym(remake_handle, "get_information");
	remake.setup = (void (*)(struct part_state *, struct part_state *, uint32_t))dlsym(remake_handle, "setup");
	remake.cleanup = (void (*)(struct part_state *))dlsym(remake_handle, "cleanup");
	remake.audio_callback = (void (*)(struct part_state *))dlsym(remake_handle, "audio_callback");
	remake.key_callback = (void (*)(struct part_state *, int, int, int, int))dlsym(remake_handle, "key_callback");
	remake.mainloop_callback = (int (*)(struct part_state *))dlsym(remake_handle, "mainloop_callback");
	if(dlerror() != NULL) {
		fprintf(stderr, "Error: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
}

/*
 *
 */
void close_remake() {
	// Close the shared library
	if(remake_handle) {
		dlclose(remake_handle);
		remake_handle = NULL;
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
