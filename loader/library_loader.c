
/* [=]===^=====================================================================================^===[=] */
// Comparison function for qsort
static int compare_release_name(const void *a, const void *b) {
	const struct loader_info *part_a = (const struct loader_info *)a;
	const struct loader_info *part_b = (const struct loader_info *)b;
	return strcmp(part_a->release_name, part_b->release_name);
}

/* [=]===^=====================================================================================^===[=] */
// Function to sort an array of part_state structs based on release_name
static void sort_by_release_name(struct loader_info *remakes, size_t remake_count) {
	qsort(remakes, remake_count, sizeof(struct loader_info), compare_release_name);
}

/* [=]===^=====================================================================================^===[=] */
void load_remakes(struct loader_state *state) {
	struct dirent *ent;

#ifdef _WIN32
	// Windows-specific variables
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	char search_path[MAX_PATH];

	snprintf(search_path, sizeof(search_path), "remakes\\remake_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			state->remake_count++;
		} while(FindNextFile(hFind, &find_data));
		FindClose(hFind);

		// Allocate memory for the remake states
		state->remakes = (struct loader_info *)calloc(state->remake_count, sizeof(struct loader_info));

		// Load the remakes
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
					strcpy_s(state->remakes[index].release_name, 40 - 1, info->release_name);
					strcpy_s(state->remakes[index].display_name, 80 - 1, info->display_name);
					strcpy_s(state->remakes[index].author_name, 40 - 1, info->author_name);

					FreeLibrary(handle);
				}
				index++;
			} while(FindNextFile(hFind, &find_data));
			FindClose(hFind);
		} else {
			printf("Failed to open remakes directory.\n");
		}
	}
#elif defined(__linux__)
	DIR *dir;

	dir = opendir("remakes");
	if(dir) {
		// Count the number of files matching the pattern
		while((ent = readdir(dir))) {
			if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
				state->remake_count++;
			}
		}
		// Allocate memory for the remake states

		state->remakes = (struct loader_info *)calloc(state->remake_count, sizeof(struct loader_info));
		rewinddir(dir);

		// Load the remakes
		int index = 0;
		while((ent = readdir(dir))) {
			if(fnmatch("remake_*.so", ent->d_name, 0) == 0) {
				snprintf(state->remakes[index].lib_path, sizeof(state->remakes[index].lib_path), "remakes/%s", ent->d_name);
				void *handle = dlopen(state->remakes[index].lib_path, RTLD_LAZY);
				if(handle) {
					struct remake_info *info = dlsym(handle, "remake_information");
					strncpy(state->remakes[index].release_name, info->release_name, 40 - 1); // These copies ensure that length does not overflow
					strncpy(state->remakes[index].display_name, info->display_name, 80 - 1);
					strncpy(state->remakes[index].author_name, info->author_name, 40 - 1);
					dlclose(handle);
				}
				index++;
			}
		}
		closedir(dir);
	} else {
		printf("Failed to open remakes directory.\n");
	}
#endif
}

/* [=]===^=====================================================================================^===[=] */
void load_selector(struct loader_state *state) {
	char path[256];
#ifdef _WIN32
	WIN32_FIND_DATA find_data = {0};
	HANDLE hFind = 0;
	char search_path[MAX_PATH] = {0};
	DWORD file_count = 0;
	char *selected_file = 0;

	snprintf(search_path, sizeof(search_path), "remakes\\selector_*.dll");
	hFind = FindFirstFile(search_path, &find_data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			file_count++;
		} while(FindNextFile(hFind, &find_data));
		FindClose(hFind);
	}

	// Randomly pick one of the files
	if(file_count > 0) {
		int selected_index = mks_rand(file_count);

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
		snprintf(path, sizeof(path), "remakes\\%s", selected_file);
		HMODULE handle = LoadLibrary(path);
		if(handle) {
			typedef struct selector_info* (*GetSelectorInfoFunc)();
			GetSelectorInfoFunc get_selector_info = (GetSelectorInfoFunc)GetProcAddress(handle, "get_selector_information");

			state->selector = get_selector_info();
			// Call the setup function
			state->shared.buffer_width = state->selector->buffer_width;
			state->shared.buffer_height = state->selector->buffer_height;

			state->selector->change_resolution = setup_texture;
			state->selector->private = state;

			if(state->selector->setup) {
				state->selector->setup(&state->shared, state->remakes, state->remake_count);
			}
			if(!state->selector->frames_per_second) {
				state->selector->frames_per_second = 50;
			}
			free(selected_file);
		}
	}
#elif defined(__linux__)
	DIR *dir = 0;
	struct dirent *ent = 0;
	int file_count = 0;
	char *selected_file = 0;

	// Count the number of files matching the pattern
	dir = opendir("remakes");
	if(dir) {
		while((ent = readdir(dir))) {
			if(fnmatch("selector_*.so", ent->d_name, 0) == 0) {
				file_count++;
			}
		}

		rewinddir(dir);
		if(file_count > 0) {
			int selected_index = mks_rand(file_count);

			// Find the selected file
			int index = 0;
			while((ent = readdir(dir))) {
				if(fnmatch("selector_*.so", ent->d_name, 0) == 0) {
					if(index == selected_index) {
						selected_file = strdup(ent->d_name);
						break;
					}
					index++;
				}
			}
		}
		closedir(dir);
	}

	// Load the selected file
	if(selected_file) {
		snprintf(path, sizeof(path), "remakes/%s", selected_file);
		void *handle = dlopen(path, RTLD_LAZY);
		if(handle) {
			state->selector = dlsym(handle, "selector_information");
			// Call the setup function
			state->shared.buffer_width = state->selector->buffer_width;
			state->shared.buffer_height = state->selector->buffer_height;

			state->selector->change_resolution = setup_texture;
			state->selector->private = state;

			if(state->selector->setup) {
				state->selector->setup(&state->shared, state->remakes, state->remake_count);
			}
			if(!state->selector->frames_per_second) {
				state->selector->frames_per_second = 50;
			}
			free(selected_file);
		}
	}
#endif
}

/* [=]===^=====================================================================================^===[=] */
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

	if(state->remake->setup) {
		state->remake->setup(&state->shared);
	}
	if(!state->shared.buffer_width || !state->shared.buffer_height) {
		state->shared.buffer_width = 368;
		state->shared.buffer_height = 272;
	}
	if(!state->remake->frames_per_second) {
		state->remake->frames_per_second = 50;
	}
}

/* [=]===^=====================================================================================^===[=] */
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
