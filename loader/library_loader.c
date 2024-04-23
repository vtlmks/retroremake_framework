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
	const struct remake_info *part_a = (const struct remake_info *)a;
	const struct remake_info *part_b = (const struct remake_info *)b;
	return strcmp(part_a->release_name, part_b->release_name);
}

// Function to sort an array of part_state structs based on release_name
static void sort_by_release_name(struct remake_info *remakes, size_t num_remakes) {
	qsort(remakes, num_remakes, sizeof(struct remake_info), compare_release_name);
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
	state->remakes = (struct loader_info *)calloc(state->remake_count, sizeof(struct loader_info));

	// Load the remakes
	snprintf(search_path, sizeof(search_path), "remakes\\remake_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if(hFind != INVALID_HANDLE_VALUE) {
		int index = 0;
		do {
			snprintf(state->remakes[index].lib_path, sizeof(state->remakes[index].lib_path), "remakes\\%s", find_data.cFileName);
			HMODULE handle = LoadLibrary(state->remakes[index].lib_path);
			if(handle) {

				typedef struct remake_info* (*GetRemakeInfoFunc)();
				GetRemakeInfoFunc get_remake_info = (GetRemakeInfoFunc)GetProcAddress(handle, "get_remake_information");

				struct remake_info *info = get_remake_info();
				strcpy_s(state->remakes[index].release_name, 40-1, info->release_name);
				strcpy_s(state->remakes[index].display_name, 80-1, info->display_name);
				strcpy_s(state->remakes[index].author_name, 40-1, info->author_name);

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
	char *selected_file = 0;

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
	if(selected_file) {
		char path[256];
		snprintf(path, sizeof(path), "remakes\\%s", selected_file);
		HMODULE handle = LoadLibrary(path);
		if(handle) {
			typedef struct selector_info* (*GetSelectorInfoFunc)();
			GetSelectorInfoFunc get_selector_info = (GetSelectorInfoFunc)GetProcAddress(handle, "get_selector_information");

			state->selector = get_selector_info();	//(struct selector_info *) GetProcAddress(state->remake_handle, "selector_information");
			// Call the setup function
			if(state->selector->setup) {
				state->selector->setup(&state->shared, state->remakes, state->remake_count);
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
		state->remakes = (struct loader_info *)calloc(state->remake_count, sizeof(struct loader_info));


		// Load the remakes
		int index = 0;
		dir = opendir("remakes");
		if(dir) {
			while((ent = readdir(dir))) {
				if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
					snprintf(state->remakes[index].lib_path, sizeof(state->remakes[index].lib_path), "remakes/%s", ent->d_name);
					void *handle = dlopen(state->remakes[index].lib_path, RTLD_LAZY);
					if(handle) {
						struct remake_info *info = dlsym(handle, "remake_information");
						strlcpy(state->remakes[index].release_name, info->release_name, 40-1);		// These copies makes sure that length does not overflow
						strlcpy(state->remakes[index].display_name, info->display_name, 80-1);
						strlcpy(state->remakes[index].author_name, info->author_name, 40-1);

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
			state->selector = dlsym(handle, "selector_information");

// 			// Call the setup function
			if(state->selector->setup) {
				state->selector->setup(&state->shared, state->remakes, state->remake_count);
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

	typedef struct remake_info* (*GetRemakeInfoFunc)();
	GetRemakeInfoFunc get_remake_info = (GetRemakeInfoFunc)GetProcAddress(state->remake_handle, "get_remake_information");
	state->remake = get_remake_info();

#elif defined(__linux__)
	// Open the shared library
	state->remake_handle = dlopen(state->remakes[index].lib_path, RTLD_LAZY);
	if(!state->remake_handle) {
		fprintf(stderr, "Error: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	state->remake = dlsym(state->remake_handle, "remake_information");
#endif

	if (state->remake->setup) {
		state->remake->setup(&state->shared);
	}
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
