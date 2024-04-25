/* ===^=====================================================================================^===[=]
 *
 *  Author: Peter Fors  aka  ViTAL
 *
 *  (C) Copyright 2023 by Mindkiller Systems, inc.
 *      All rights reserved.
 *
 *       _\____ ___ __  _                                      _ __ ___ ____/_
 *         \     _ _    .    ___   ___|   |_,   .   |   |    __     __     /
 *     _ ___\ - ( ` ) - | - (  / - \__) - ( \ - | - | - | - (__/ - (  ` - /___
 *  _\________        .      ________    _____     _________`-__  ____ ________/_
 *   _\  ____/___:::: ::::  _\  ____/____\  _/__  _\_  _____/\_ \/  _/_\  ____/___
 *   \________  /::::.:.... \________  /  _____/_/  _____/__  _  _    \________  /
 *   /  __  /  /....  ::::_ /  __  /_      /  _ /   /_  _      \/  _ /   __  /  /
 *  \____/    /_ :::: :::: \____/   _\_____    \_____   _\____ /   _\_____/    /_
 *  jA!_/____/// `:::.::::. .  /____/   _/____/    /____/     /____/e^d _/____///
 *     /              :                 /                               /
 *                    .
 *                        M i n d k i l l e r   S y s t e m s
 *  Version 0.1
 *
 *
 *
 * “The ships hung in the sky in much the same way that bricks don't.” ― Douglas Adams
 *
 */


//         -  - -- --- ---- -----=<{[ includes ]}>=----- ---- --- -- -  -

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <time.h>
#include <GL/glx.h>
#endif

#include "glcorearb.h"

#include "opengl.c"
#include "rand.c"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Default scale
#define SCALE (3)

#include "misc.h"

#include "fragment_shader.h"
#include "vertex_shader.h"

#include "loader.h"
#include "remake.h"
#include "selector.h"

#include "loader_internal.h"

#include "audio.c"
#include "shader.c"

#include "library_loader.c"

/* [=]===^=====================================================================================^===[=] */
// NOTE(peter): We can steal F11 and F12 here, and the shift, ctrl, alt, version of them, they are not on the Amiga keyboard.
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	static int window_pos[2];
	static int window_size[2];

	struct loader_state *state = glfwGetWindowUserPointer(window);

	// NOTE(peter): Toggle CRT emulation
	if(key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		state->toggle_crt_emulation = !state->toggle_crt_emulation;
	}

	// NOTE(peter): Toggle fullscreen
	if(glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS) {
		if(glfwGetWindowMonitor(window) == 0) {

			glfwGetWindowPos(window, &window_pos[0], &window_pos[1]);
			glfwGetWindowSize(window, &window_size[0], &window_size[1]);

			GLFWmonitor *primary = glfwGetPrimaryMonitor();
			const GLFWvidmode *mode = glfwGetVideoMode(primary);
			glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);

		} else {
			glfwSetWindowMonitor(window, 0, window_pos[0], window_pos[1], window_size[0], window_size[1], 0);
		}
	}

	// NOTE(peter): Update the keyboard_state
	if(action == GLFW_PRESS) {
		state->shared.keyboard_state[key] = 1;
	} else if(action == GLFW_RELEASE) {
		state->shared.keyboard_state[key] = 0;
	}

	// NOTE(peter): Test code to switch between SELECTOR and a REMAKE, this will stop working when I remove the hardcoded
	if(action == GLFW_RELEASE) {
		if(key == GLFW_KEY_2) {
			state->mode = LOAD_REMAKE_MODE;
		} else if(key == GLFW_KEY_1) {
			state->mode = SELECTOR_MODE;
		}
	}

	// NOTE(peter): Call the key_callback of the selector or remake if it is defined.
	switch(state->mode) {
		case REMAKE_MODE: {
			if(state->remake->key_callback) {
				state->remake->key_callback(&state->shared, key);
			}
		} break;
		case SELECTOR_MODE: {
			if(state->selector->key_callback) {
				state->selector->key_callback(&state->shared, key);
			}
		} break;
	}
}

/* [=]===^=====================================================================================^===[=] */
static void framebuffer_callback(GLFWwindow *window, int width, int height) {
	struct loader_state *state = glfwGetWindowUserPointer(window);

	const float wanted_aspect = 4.f/3.f;
	float current_aspect = (float)width/(float)height;

	state->viewport.x = 0.0f;
	state->viewport.y = 0.0f;
	state->viewport.w = width;
	state->viewport.h = height;

	if(current_aspect > wanted_aspect) {		// NOTE(peter): If we are fullscreen
		float new_width = width * (wanted_aspect / current_aspect);
		state->viewport.x = (width - new_width) / 2;
		state->viewport.w = new_width;
	}
}

/* [=]===^=====================================================================================^===[=] */
static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	struct loader_state *state = glfwGetWindowUserPointer(window);

	int windowWidth, windowHeight;
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	// Calculate the ratio of window size to buffer size
	double widthRatio = (double)windowWidth / BUFFER_WIDTH;
	double heightRatio = (double)windowHeight / BUFFER_HEIGHT;

	state->shared.mouse_x = (int)(xpos / widthRatio);
	state->shared.mouse_y = (int)(ypos / heightRatio);
	// printf("x: %d  y: %d\n", state->shared.mouse_x, state->shared.mouse_y);
}

/* [=]===^=====================================================================================^===[=] */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	struct loader_state *state = glfwGetWindowUserPointer(window);

	// NOTE(peter): Update the mouse_button_state
	if(action == GLFW_PRESS) {
		state->shared.mouse_button_state[button] = 1;
	} else if(action == GLFW_RELEASE) {
		state->shared.mouse_button_state[button] = 0;
	}

	if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		state->shared.grab_cursor = !state->shared.grab_cursor;
	// } else if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
	// 	state->shared.mouse_button_state[button] = 0;
	}

}

/* [=]===^=====================================================================================^===[=] */
static void error_callback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

/* [=]===^=====================================================================================^===[=] */
int main(int argc, char **argv) {
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint texture;
	GLFWwindow *window;
	char window_title[512];

	struct loader_state state = {0};
	state.toggle_crt_emulation = true;

	load_remakes(&state);
	load_selector(&state);

	load_remake(&state, 0);		// TODO(peter): Remove me, this should be done after the selector has selected a remake, or if the loader was started with an argument to load remake directly

#ifdef _WIN32
	timeBeginPeriod(1);
	SetPriorityClass(0, HIGH_PRIORITY_CLASS);
#else
	setbuf(stdout, 0);	// flush immediately to stdout
#endif

	// TODO(peter): Setup a dummy callback that output just zeroes to the buffer...
	audio_initialize(&state);
	glfwSetErrorCallback(error_callback);

	if(glfwInit()) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// NOTE(peter): Only for 3.2+

		uint32_t scaled_window_width = BUFFER_HEIGHT * SCALE * (4.f/3.f);
		uint32_t scaled_window_height = BUFFER_HEIGHT * SCALE;

		uint32_t min_window_width = BUFFER_HEIGHT * 2 * (4.f/3.f);
		uint32_t min_window_height = BUFFER_HEIGHT * 2;

		if((window = glfwCreateWindow(scaled_window_width, scaled_window_height, "This will change when remake/selector is loaded", 0, 0))) {
			snprintf(window_title, sizeof(window_title), "%s - %s", state.selector->window_title, "Middle Mouse to release mouse - ESC to Exit");
			glfwSetWindowTitle(window, window_title);
			glfwMakeContextCurrent(window);
			gl_init(&opengl);
			glfwSwapInterval(0);																	// NOTE(peter): We handle frame-synchronisation ourself.

			if(glfwRawMouseMotionSupported()) {
				glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
			}

			glfwSetKeyCallback(window, key_callback);
			glfwSetCursorPosCallback(window, mouse_move_callback);
			glfwSetMouseButtonCallback(window, mouse_button_callback);
			glfwSetFramebufferSizeCallback(window, framebuffer_callback);
			glfwSetWindowAspectRatio(window, 4, 3);
			glfwSetWindowSizeLimits(window, min_window_width, min_window_height, -1, -1);
			glfwSetWindowUserPointer(window, &state);
			glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);		// NOTE(peter): Sticky mouse buttons, so we don't miss any events..

			state.shared.buffer = malloc(BUFFER_WIDTH * BUFFER_HEIGHT * sizeof(uint32_t));

			state.viewport.x = 0;
			state.viewport.y = 0;
			state.viewport.w	= scaled_window_width;
			state.viewport.h = scaled_window_height;

			// Setup Shader
			GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader, 1, &vertex_shader_start, 0);
			glCompileShader(vertex_shader);
			GLint success;
			GLchar infoLog[512];
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(vertex_shader, 512, 0, infoLog);
				printf("Vertex shader compilation failed:\n%s", infoLog);
			}
			GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &fragment_shader_start, 0);

			glCompileShader(fragment_shader);
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(fragment_shader, 512, 0, infoLog);
				printf("Fragment shader compilation failed:\n%s", infoLog);
			}

			shader_program = glCreateProgram();
			glAttachShader(shader_program, vertex_shader);
			glAttachShader(shader_program, fragment_shader);
			glLinkProgram(shader_program);
			int uniform_resolution		= glGetUniformLocation(shader_program, "resolution");
			int uniform_src_image_size	= glGetUniformLocation(shader_program, "src_image_size");
			int uniform_saturation		= glGetUniformLocation(shader_program, "saturation");
			int uniform_brightness		= glGetUniformLocation(shader_program, "brightness");
			int uniform_tone				= glGetUniformLocation(shader_program, "tone_data");
			int uniform_crt_emulation	= glGetUniformLocation(shader_program, "crt_emulation");
			glUseProgram(shader_program);
			glDeleteShader(vertex_shader);
			glDeleteShader(fragment_shader);

			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ebo);

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 * sizeof(float)));			// position attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));			// texture coord attribute
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);                        			 // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // set texture wrapping to GL_REPEAT (default wrapping method)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, state.shared.buffer);

			bool running = true;

			float contrast = 1.0f;
			float saturation = 0.0f;
			float brightness = 1.2f;

			/*
			 * Move this into the mainloop if change of contrast/saturation is added as an interactive thing.
			 */
			float tone_dat[4] = {0.f};
			CrtsTone(tone_dat, contrast, saturation, INPUT_THIN, INPUT_MASK);

			double next_update = glfwGetTime() + FRAME_TIME;

			while(running && !glfwWindowShouldClose(window)) {
				glfwPollEvents();

				// NOTE(peter): this may be temporary, check if we are to exit.
				if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ||
					(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)) {
					glfwSetWindowShouldClose(window, true);
				}

				// NOTE(peter): Handle grabbing cursor
				if(state.shared.grab_cursor != state.cursor_locked) {
					state.cursor_locked = state.shared.grab_cursor;
					if(state.shared.grab_cursor) {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					} else {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					}
				}

				switch(state.mode) {
					case SELECTOR_MODE: {
						if(state.selector->mainloop_callback) {
							state.selector->mainloop_callback(&state.shared);
						}
					} break;
					case LOAD_REMAKE_MODE: {
						// load_remake(&state, state->selected_mode);
// BUG: state.remake.window_title is not filled in, make sure that we call get_information when we load a remake.

						snprintf(window_title, sizeof(window_title), "%s - %s", state.remakes[0].release_name, "Middle Mouse to release mouse - ESC to Exit");
						glfwSetWindowTitle(window, window_title);
						state.mode = REMAKE_MODE;
					} break;
					case REMAKE_MODE: {
						if(state.remake->mainloop_callback) {
							state.remake->mainloop_callback(&state.shared);
						}
					} break;
					case UNLOAD_REMAKE_MODE: {
						snprintf(window_title, sizeof(window_title), "%s - %s", state.selector->window_title, "Middle Mouse to release mouse - ESC to Exit");
						state.mode = SELECTOR_MODE;
					} break;
				}

				// NOTE(peter): Rendering stuff
				glViewport(state.viewport.x, state.viewport.y, state.viewport.w, state.viewport.h);
				glClearColor(0.f, 0.f, 0.f, 0.f);
				glClear(GL_COLOR_BUFFER_BIT);

				// These values are set once, move inside the mainloop if you want to vary any one of them.
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, state.shared.buffer);

				glUseProgram(shader_program);
				glUniform2f(uniform_src_image_size, (float)BUFFER_WIDTH, (float)BUFFER_HEIGHT);
				glUniform2f(uniform_resolution, state.viewport.w, state.viewport.h);
				glUniform1f(uniform_brightness, brightness);
				glUniform4f(uniform_tone, tone_dat[0], tone_dat[1], tone_dat[2], tone_dat[3]);
				glUniform1i(uniform_crt_emulation, state.toggle_crt_emulation);
				glBindVertexArray(vao);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
				glDrawElements(GL_TRIANGLES, ARRAY_SIZE(indices), GL_UNSIGNED_INT, 0);

				glfwSwapBuffers(window);

				/*
				* NOTE(peter): The granularity of sleep is not great on any platform, necessitating a combination
				* of sleeping until <1ms remains and then spin-looping until we reach the desired time.
				*
				* This task should ideally be the last operation in a frame to ensure that other rendering and
				* processing tasks are completed before pacing the frame rate.
				*/

				double remaining_time = next_update - glfwGetTime();
				if(remaining_time > 0.0) {											// Do we need to wait?
					if(remaining_time > 0.001) {									// sleep, if needed, until less than 1ms left
						mks_sleep(remaining_time - 0.001);
					}
					while((next_update - glfwGetTime()) > 0.0) { }			// wait <1ms
				} else { next_update = glfwGetTime(); }						// We are in the future, next_updates is set to now to not fastforward
				next_update += FRAME_TIME;

				state.shared.frame_number++;
				// part_state.ms_since_start += (uint32_t)(1000.f / FRAMES_PER_SECOND);
			}

			glDeleteProgram(shader_program);
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ebo);
			glDeleteTextures(1, &texture);
			free(state.shared.buffer);
			glfwDestroyWindow(window);
		} else {
			printf("ERROR: GLFW could not create window!\n");
		}
		glfwTerminate();
	} else {
		printf("ERROR: Could not initialize glfw!\n");
	}
	audio_shutdown();

#ifdef _WIN32
	timeEndPeriod(1);
#endif
	return 0;
}
