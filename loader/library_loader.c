/*
 *
 */

static int match(char *start, char *end, const char *filename) {
	return !(strncmp(filename, start, strlen(start)) == 0 && strstr(filename, end) != 0);
}

typedef struct directory_handle directory_handle;

#ifdef _WIN32
static char *library_ending = ".dll";

static void *library_open(const char *path) {
	return LoadLibrary(path);
}

static void library_close(void *handle) {
	FreeLibrary((HMODULE)handle);
}

static void *library_get_symbol(void *handle, const char *symbol) {
	return GetProcAddress((HMODULE)handle, symbol);
}

static void strlcpy(char *dst, const char *src, size_t dst_size) {
	if(dst_size == 0) return;
	while(--dst_size && (*dst++ = *src++));
	*dst = '\0';
}

// Directory handling

struct directory_handle {
	HANDLE hFind;
	WIN32_FIND_DATA find_data;
	char search_path[MAX_PATH];
	int first_entry;
};

static directory_handle *open_directory(const char *path) {
	directory_handle *handle = malloc(sizeof(directory_handle));
	snprintf(handle->search_path, sizeof(handle->search_path), "%s\\*", path);
	handle->hFind = FindFirstFile(handle->search_path, &handle->find_data);
	handle->first_entry = 1;
	if(handle->hFind == INVALID_HANDLE_VALUE) {
		free(handle);
		return 0;
	}
	return handle;
}

static void close_directory(directory_handle *handle) {
	if(handle) {
		if(handle->hFind != INVALID_HANDLE_VALUE) {
			FindClose(handle->hFind);
		}
		free(handle);
	}
}

static const char *read_next_file(directory_handle *handle) {
	if(!handle) return 0;
	if(handle->first_entry) {
		handle->first_entry = 0;
		return handle->find_data.cFileName;
	}
	if(FindNextFile(handle->hFind, &handle->find_data)) {
		return handle->find_data.cFileName;
	}
	return 0;
}

#else
static char *library_ending = ".so";

static void *library_open(const char *path) {
	return dlopen(path, RTLD_LAZY);
}

static void library_close(void *handle) {
	dlclose(handle);
}

static void *library_get_symbol(void *handle, const char *symbol) {
	return dlsym(handle, symbol);
}

// Directory handling

struct directory_handle {
	DIR *dir;
	struct dirent *entry;
	char path[256];
};

static directory_handle *open_directory(const char *path) {
	directory_handle *handle = malloc(sizeof(directory_handle));
	if(!handle) return 0;
	handle->dir = opendir(path);
	if(!handle->dir) {
		free(handle);
		return 0;
	}
	strncpy(handle->path, path, sizeof(handle->path));
	return handle;
}

static void close_directory(directory_handle *handle) {
	if(handle) {
		if(handle->dir) {
			closedir(handle->dir);
		}
		free(handle);
	}
}

static const char *read_next_file(directory_handle *handle) {
	if(!handle || !handle->dir) return 0;
	handle->entry = readdir(handle->dir);
	if(handle->entry) {
		return handle->entry->d_name;
	}
	return 0;
}
#endif

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
static void load_remakes(struct loader_state *state) {

	directory_handle *dir = open_directory("remakes");
	if(dir) {
		const char *filename;
		while((filename = read_next_file(dir))) {
			if(match("remake_", library_ending, filename) == 0) {
				state->remake_count++;
			}
		}
		state->remakes = (struct loader_info *)calloc(state->remake_count, sizeof(struct loader_info));
		close_directory(dir);

		dir = open_directory("remakes");
		int index = 0;
		while((filename = read_next_file(dir))) {
			if(match("remake_", library_ending, filename) == 0) {
				snprintf(state->remakes[index].lib_path, sizeof(state->remakes[index].lib_path), "remakes/%s", filename);
				void *handle = library_open(state->remakes[index].lib_path);
				if(handle) {
					typedef struct remake_info* (*GetRemakeInfoFunc)();
					GetRemakeInfoFunc get_remake_info = (GetRemakeInfoFunc)library_get_symbol(handle, "get_remake_information");
					struct remake_info *info = get_remake_info();
					strlcpy(state->remakes[index].release_name, info->release_name, sizeof(state->remakes[index].release_name));
					strlcpy(state->remakes[index].display_name, info->display_name, sizeof(state->remakes[index].display_name));
					strlcpy(state->remakes[index].author_name, info->author_name, sizeof(state->remakes[index].author_name));
					library_close(handle);
				}
				index++;
			}
		}
		close_directory(dir);
	} else {
		printf("Failed to open remakes directory.\n");
	}
}

/* [=]===^=====================================================================================^===[=] */
static void load_selector(struct loader_state *state) {
	const char *filename;
	uint32_t file_count = 0;
	char *selected_file = 0;
	char path[256];

	directory_handle *dir = open_directory("remakes");
	if(dir) {
		while((filename = read_next_file(dir))) {
			if(match("selector_", library_ending, filename) == 0) {
				file_count++;
			}
		}
		close_directory(dir);

		if(file_count) {
			int selected_index = mks_rand(file_count);
			int index = 0;
			dir = open_directory("remakes");
			while((filename = read_next_file(dir))) {
				if(match("selector_", library_ending, filename) == 0) {
					if(index == selected_index) {
						selected_file = strdup(filename);
						break;
					}
					index++;
				}
			}
			close_directory(dir);
		}
	}

	if(selected_file) {
		snprintf(path, sizeof(path), "remakes/%s", selected_file);
		void *handle = library_open(path);
		if(handle) {
			typedef struct selector_info* (*GetSelectorInfoFunc)();
			GetSelectorInfoFunc get_selector_info = (GetSelectorInfoFunc)library_get_symbol(handle, "get_selector_information");
			state->selector = get_selector_info();

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
}

/* [=]===^=====================================================================================^===[=] */
static void load_remake(struct loader_state *state, uint32_t index) {

	state->remake_handle = library_open(state->remakes[index].lib_path);

	if(!state->remake_handle) {
		fprintf(stderr, "Error: Unable to load remake.\n");
		exit(EXIT_FAILURE);
	}
	typedef struct remake_info* (*GetRemakeInfoFunc)();
	GetRemakeInfoFunc get_remake_info = (GetRemakeInfoFunc)library_get_symbol(state->remake_handle, "get_remake_information");
	state->remake = get_remake_info();

	state->remake->change_resolution = setup_texture;
	state->remake->private = state;

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

static void close_remake(struct loader_state *state) {
	if(state->remake_handle) {
		library_close(state->remake_handle);
		state->remake_handle = 0;
	}
}
