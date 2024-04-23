#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

#elif defined(__linux__)
#include <dirent.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <time.h>

#endif




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


#ifdef _WIN32

/*
 *  :::       ::: ::::::::::: ::::    ::: :::::::::   ::::::::  :::       :::  ::::::::
 *  :+:       :+:     :+:     :+:+:   :+: :+:    :+: :+:    :+: :+:       :+: :+:    :+:
 *  +:+       +:+     +:+     :+:+:+  +:+ +:+    +:+ +:+    +:+ +:+       +:+ +:+
 *  +#+  +:+  +#+     +#+     +#+ +:+ +#+ +#+    +:+ +#+    +:+ +#+  +:+  +#+ +#++:++#++
 *  +#+ +#+#+ +#+     +#+     +#+  +#+#+# +#+    +#+ +#+    +#+ +#+ +#+#+ +#+        +#+
 *   #+#+# #+#+#      #+#     #+#   #+#+# #+#    #+# #+#    #+#  #+#+# #+#+#  #+#    #+#
 *    ###   ###   ########### ###    #### #########   ########    ###   ###    ########
 */

/*
 * TODO(peter): Fix documentation for this.
 */

void load_remakes(struct loader_state *state) {
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	char search_path[MAX_PATH];

	snprintf(search_path, sizeof(search_path), "remakes\\remake_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do{
			state->remake_count++;
		} while(FindNextFile(hFind, &find_data));
		FindClose(hFind);
	}

	// Allocate memory for the remake states
	state->remakes = (struct remake_state *)calloc(state->remake_count, sizeof(struct remake_state));

	// Load the remakes
	snprintf(search_path, sizeof(search_path), "remakes\\remake_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if(hFind != INVALID_HANDLE_VALUE) {
		int index = 0;
		do {
			snprintf(state->remakes[index].lib_path, sizeof(state->remakes[index].lib_path), "remakes\\%s", find_data.cFileName);
			HMODULE handle = LoadLibrary(state->remakes[index].lib_path);
			if(handle != NULL) {
				void (*get_info)(struct remake_state *) = (void (*)(struct remake_state *))GetProcAddress(handle, "get_information");
				if(get_info) {
					get_info(&state->remakes[index]);
				}
				FreeLibrary(handle);
			}
			index++;
		} while(FindNextFile(hFind, &find_data));
		FindClose(hFind);
	} else {
		printf("Failed to open remakes directory.\n");
	}
}

void load_selector(struct loader_state *state) {
	WIN32_FIND_DATA find_data = {0};
	HANDLE hFind = 0;
	char search_path[MAX_PATH] = {0};
	DWORD num_files = 0;
	char *selected_file = NULL;

	snprintf(search_path, sizeof(search_path), "remakes\\selector_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			num_files++;
		} while(FindNextFile(hFind, &find_data));
		FindClose(hFind);
	}

	// Randomly pick one of the files
	if(num_files > 0) {
		srand(time(0));
		int selected_index = rand() % num_files;

		// Find the selected file
		int index = 0;
		hFind = FindFirstFile(search_path, &find_data);
		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				if(index == selected_index) {
					selected_file = _strdup(find_data.cFileName);
					break;
				}
				index++;
			} while(FindNextFile(hFind, &find_data));
			FindClose(hFind);
		}
	}

	// Load the selected file
	if(selected_file != NULL) {
		char path[256];
		snprintf(path, sizeof(path), "remakes\\%s", selected_file);
		HMODULE handle = LoadLibrary(path);
		if(handle != NULL) {
			// Get pointers to functions in the struct and store them in the global 'selector' struct
			state->selector.setup = (void (*)(struct selector_state *, struct remake_state *, uint32_t))GetProcAddress(handle, "setup");
			state->selector.cleanup = (void (*)(struct loader_shared_state *))GetProcAddress(handle, "cleanup");
			state->selector.audio_callback = (void (*)(struct loader_shared_state *, int16_t *, size_t))GetProcAddress(handle, "audio_callback");
			state->selector.key_callback = (void (*)(struct loader_shared_state *, int))GetProcAddress(handle, "key_callback");
			state->selector.mainloop_callback = (int (*)(struct loader_shared_state *))GetProcAddress(handle, "mainloop_callback");

			// Call the setup function
			if(state->selector.setup != NULL) {
				state->selector.setup(&state->selector, state->remakes, state->remake_count);
			}
			free(selected_file);
		}
	}
}


#elif defined(__linux__)

/*
 *            :::        ::::::::::: ::::    ::: :::    ::: :::    :::
 *            :+:            :+:     :+:+:   :+: :+:    :+: :+:    :+:
 *            +:+            +:+     :+:+:+  +:+ +:+    +:+  +:+  +:+
 *            +#+            +#+     +#+ +:+ +#+ +#+    +:+   +#++:+
 *            +#+            +#+     +#+  +#+#+# +#+    +#+  +#+  +#+
 *            #+#            #+#     #+#   #+#+# #+#    #+# #+#    #+#
 *            ########## ########### ###    ####  ########  ###    ###
 */

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
	DIR *dir = 0;
	struct dirent *ent = 0;
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
				state->selector.setup(&state->selector, state->remakes, state->remake_count);
			}
			free(selected_file);
		}
	}
}


#endif

/*
 *
 */
static void load_remake(struct loader_state *state, uint32_t index) {
#ifdef _WIN32
	char search_path[MAX_PATH];
	WIN32_FIND_DATA find_data;
	HANDLE hFind;

	snprintf(search_path, sizeof(search_path), "%s", state->remakes[index].lib_path);
	state->remake_handle = LoadLibrary(search_path);
	if(!state->remake_handle) {
		fprintf(stderr, "Error: Unable to load remake.\n");
		exit(EXIT_FAILURE);
	}

	// Get function pointers using GetProcAddress() and store them in the global variable 'remake'
	// remake.get_information = (void (*)(struct remake_state *))GetProcAddress(remake_handle, "get_information");
	state->remake.setup = (void (*)(struct loader_shared_state *))GetProcAddress(state->remake_handle, "setup");
	state->remake.cleanup = (void (*)(struct loader_shared_state *))GetProcAddress(state->remake_handle, "cleanup");
	state->remake.audio_callback = (void (*)(struct loader_shared_state *, int16_t *, size_t))GetProcAddress(state->remake_handle, "audio_callback");
	state->remake.key_callback = (void (*)(struct loader_shared_state *, int))GetProcAddress(state->remake_handle, "key_callback");
	state->remake.mainloop_callback = (int (*)(struct loader_shared_state *))GetProcAddress(state->remake_handle, "mainloop_callback");

#elif defined(__linux__)
	// Open the shared library
	state->remake_handle = dlopen(state->remakes[index].lib_path, RTLD_LAZY);
	if(!state->remake_handle) {
		fprintf(stderr, "Error: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	// Get function pointers using dlsym() and store them in the global variable 'remake'
	// state->remake.get_information = (void (*)(struct remake_state *))dlsym(remake_handle, "get_information");
	state->remake.setup = (void (*)(struct loader_shared_state *))dlsym(state->remake_handle, "setup");
	state->remake.cleanup = (void (*)(struct loader_shared_state *))dlsym(state->remake_handle, "cleanup");
	state->remake.audio_callback = (void (*)(struct loader_shared_state *, int16_t *, size_t))dlsym(state->remake_handle, "audio_callback");
	state->remake.key_callback = (void (*)(struct loader_shared_state *, int))dlsym(state->remake_handle, "key_callback");
	state->remake.mainloop_callback = (int (*)(struct loader_shared_state *))dlsym(state->remake_handle, "mainloop_callback");
#endif

	if (!state->remake.setup ||
		!state->remake.cleanup ||
		!state->remake.audio_callback ||
		!state->remake.key_callback ||
		!state->remake.mainloop_callback) {
		fprintf(stderr, "Error: Unable to get all function pointers.\n");
		exit(EXIT_FAILURE);
	}
	state->remake.setup(&state->shared);
}


/*
 *
 */
static void close_remake(struct loader_state *state) {
	// Close the shared library
	if(state->remake_handle) {
#ifdef _WIN32
		FreeLibrary(state->remake_handle);
#elif defined(__linux__)
		dlclose(state->remake_handle);
#endif
		state->remake_handle = NULL;
	}
}
