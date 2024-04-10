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
 */

// “The ships hung in the sky in much the same way that bricks don't.” ― Douglas Adams

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>

#include <misc.h>

#define SOGL_MAJOR_VERSION 3
#define SOGL_MINOR_VERSION 1
#ifdef _WIN32
#define SOGL_IMPLEMENTATION_WIN32 /* or SOGL_IMPLEMENTATION_X11 */
#elif __linux__
#define SOGL_IMPLEMENTATION_X11 /* or SOGL_IMPLEMENTATION_X11 */
#endif
#include "gl3w.h"
#include "gl3w.c"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// NOTE(peter): This is the default window scale.
#define SCALE (3)

#include "audio.c"

#ifdef _WIN32
	#pragma comment(lib, "winmm.lib")
	#pragma comment(lib, "ntdll.lib")
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

#include "fragment_shader.h"
#include "vertex_shader.h"

#include "shader.c"

#include "loader.h"
static uint32_t buffer[BUFFER_WIDTH * BUFFER_HEIGHT];
#include "library_loader.c"

static const char* glsl_version = "#version 140";
static const float vertices[] = {
	 1.f,-1.f, 0.0f, 1.f, 0.f,
	 1.f, 1.f, 0.0f, 1.f, 1.f,
	-1.f, 1.f, 0.0f, 0.f, 1.f,
	-1.f,-1.f, 0.0f, 0.f, 0.f
};
static const uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };

static int window_pos[2];
static int window_size[2];
static uint32_t toggle_crt_emulation = true;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if(key == GLFW_KEY_C && action == GLFW_PRESS) {
		toggle_crt_emulation = !toggle_crt_emulation;
	}

	if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS &&
		glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
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

	// struct base_state *state = glfwGetWindowUserPointer(window);
	// if(key_callback) {
	// 	key_callback(state, key, scancode, action, mods);
	// }
}

static void error_callback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

struct viewport { float x, y, w, h; };
static struct viewport remake_viewport;

static void framebuffer_callback(GLFWwindow *window, int width, int height) {

	const float wanted_aspect = 4.f/3.f;
	float current_aspect = (float)width/(float)height;

	remake_viewport.x = 0.0f;
	remake_viewport.y = 0.0f;
	remake_viewport.w = width;
	remake_viewport.h = height;

	if(current_aspect > wanted_aspect) {		// NOTE(peter): If we are fullscreen
		float new_width = width * (wanted_aspect / current_aspect);
		remake_viewport.x = (width - new_width) / 2;
		remake_viewport.w = new_width;
	}
}

// [=]===^=====================================================================================^===[=]
int main(int argc, char **argv) {
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint texture;
	int uniform_resolution;
	int uniform_src_image_size;
	int uniform_contrast;
	int uniform_saturation;
	int uniform_brightness;
	int uniform_tone;
	int uniform_crt_emulation;
	bool running;
	GLFWwindow *window;

	load_remakes();
	load_selector();

#ifdef _WIN32
	timeBeginPeriod(1);
	SetPriorityClass(0, HIGH_PRIORITY_CLASS);
#else
	setbuf(stdout, 0);	// flush immediately to stdout
#endif

	printf("audio: %d\n", setup_audio());
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
			glfwMakeContextCurrent(window);
			gl3wInit();
			glfwSwapInterval(0);																	// NOTE(peter): We handle frame-synchronisation ourself.

			glfwSetKeyCallback(window, key_callback);
			glfwSetFramebufferSizeCallback(window, framebuffer_callback);
			glfwSetWindowAspectRatio(window, 4, 3);
			glfwSetWindowSizeLimits(window, min_window_width, min_window_height, -1, -1);
			// glfwSetWindowUserPointer(window, state);
			glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);		// NOTE(peter): Sticky mouse buttons, so we don't miss any events..

			remake_viewport.x = 0;
			remake_viewport.y = 0;
			remake_viewport.w	= scaled_window_width;
			remake_viewport.h = scaled_window_height;

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
			uniform_resolution		= glGetUniformLocation(shader_program, "resolution");
			uniform_src_image_size	= glGetUniformLocation(shader_program, "src_image_size");
			uniform_contrast			= glGetUniformLocation(shader_program, "contrast");
			uniform_saturation		= glGetUniformLocation(shader_program, "saturation");
			uniform_brightness		= glGetUniformLocation(shader_program, "brightness");
			uniform_tone				= glGetUniformLocation(shader_program, "tone_data");
			uniform_crt_emulation	= glGetUniformLocation(shader_program, "crt_emulation");
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
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer);

			running = true;

			float contrast = 1.0f;
			float saturation = 0.0f;
			float brightness = 1.0f;

			/*
			 * Move this into the mainloop if change of contrast/saturation is added as an interactive thing.
			 */
			float tone_dat[4] = {0.f};
			CrtsTone(tone_dat, contrast, saturation, INPUT_THIN, INPUT_MASK);

			double next_update = glfwGetTime() + FRAME_TIME;

			glfwSetWindowTitle(window, selector.window_title);
			selector.buffer = buffer;
			for(uint32_t i = 0; i < num_remakes; ++i) {
				remakes[i].buffer = buffer;
			}

			while(running && !glfwWindowShouldClose(window)) {
				glfwPollEvents();

	printf("%d\n", cal);

				if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ||
					(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)) {
					glfwSetWindowShouldClose(window, true);
				}

uint32_t demo_state = 0;

				switch(demo_state) {
					case 0: {
						selector.mainloop_callback(&selector);
					} break;
					case 1: {
						// demo_mainloop();
					}
				}

				// NOTE(peter): Rendering stuff
				glViewport(remake_viewport.x, remake_viewport.y, remake_viewport.w, remake_viewport.h);
				glClearColor(0.f, 0.f, 0.f, 0.f);
				glClear(GL_COLOR_BUFFER_BIT);
				// These values are set once, move inside the mainloop if you want to vary any one of them.
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer);

				glUseProgram(shader_program);
				glUniform2f(uniform_src_image_size, (float)BUFFER_WIDTH, (float)BUFFER_HEIGHT);
				glUniform2f(uniform_resolution, remake_viewport.w, remake_viewport.h);
				glUniform1f(uniform_brightness, brightness);
				glUniform1f(uniform_contrast, contrast);
				glUniform4f(uniform_tone, tone_dat[0], tone_dat[1], tone_dat[2], tone_dat[3]);
				glUniform1i(uniform_crt_emulation, toggle_crt_emulation);
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

				// part_state.frame_number++;
				// part_state.ms_since_start += (uint32_t)(1000.f / FRAMES_PER_SECOND);
			}
			glfwDestroyWindow(window);
		} else {
			printf("ERROR: GLFW could not create window!\n");
		}
		glfwTerminate();
	} else {
		printf("ERROR: Could not initialize glfw!\n");
	}
#ifdef _WIN32
	timeEndPeriod(1);
#endif
	return 0;
}
