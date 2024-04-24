#pragma once

// IMPORTANT(peter): DO NOT CHANGE THESE!!!
//                   There is nothing that checks these values, they are for optimization/convenience only
#define BUFFER_WIDTH (368)
#define BUFFER_HEIGHT (276)
#define FRAMES_PER_SECOND 50.0
#define FRAME_TIME (1.0/FRAMES_PER_SECOND);

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

/*
 * TODO(peter): joypad, we need to emulate joy on keyboard for people who has no joystick and
 *              want to enter hiddenpart via joystick firebutton
 */

/*
 * -=[*]=- part_state struct documentation
 */


struct loader_info {
	char lib_path[256];
	char release_name[40];
	char display_name[80];
	char author_name[40];
};

struct loader_shared_state {
	uint32_t *buffer;						// This is the screen, [BUFFER_WIDTH * BUFFER_HEIGHT] in size, RGBA format.
	void *remake_userdata;				// This can be filled in with a pointer to a struct containing data for the selector/remake.
	void *selector_userdata;			// This can be filled in with a pointer to a struct containing data for the selector/remake.
	char keyboard_state[512];			// You can check in this array what keys are pressed, they are defined below!
	char mouse_button_state[8];
	int32_t mouse_x;
	int32_t mouse_y;
	int32_t frame_number;
	bool grab_cursor;						// lock cursor to window.
};

#define REMAKE_MOUSE_BUTTON_LEFT		0
#define REMAKE_MOUSE_BUTTON_RIGHT		1
#define REMAKE_MOUSE_BUTTON_MIDDLE	2	// cant be used, as the main program use it to lock the cursor to the window when pressed, to make mousemovement work nicely inside remakes.

#define REMAKE_KEY_SPACE              32
#define REMAKE_KEY_APOSTROPHE         39  /* ' */
#define REMAKE_KEY_COMMA              44  /* , */
#define REMAKE_KEY_MINUS              45  /* - */
#define REMAKE_KEY_PERIOD             46  /* . */
#define REMAKE_KEY_SLASH              47  /* / */
#define REMAKE_KEY_0                  48
#define REMAKE_KEY_1                  49
#define REMAKE_KEY_2                  50
#define REMAKE_KEY_3                  51
#define REMAKE_KEY_4                  52
#define REMAKE_KEY_5                  53
#define REMAKE_KEY_6                  54
#define REMAKE_KEY_7                  55
#define REMAKE_KEY_8                  56
#define REMAKE_KEY_9                  57
#define REMAKE_KEY_SEMICOLON          59  /* ; */
#define REMAKE_KEY_EQUAL              61  /* = */
#define REMAKE_KEY_A                  65
#define REMAKE_KEY_B                  66
#define REMAKE_KEY_C                  67
#define REMAKE_KEY_D                  68
#define REMAKE_KEY_E                  69
#define REMAKE_KEY_F                  70
#define REMAKE_KEY_G                  71
#define REMAKE_KEY_H                  72
#define REMAKE_KEY_I                  73
#define REMAKE_KEY_J                  74
#define REMAKE_KEY_K                  75
#define REMAKE_KEY_L                  76
#define REMAKE_KEY_M                  77
#define REMAKE_KEY_N                  78
#define REMAKE_KEY_O                  79
#define REMAKE_KEY_P                  80
#define REMAKE_KEY_Q                  81
#define REMAKE_KEY_R                  82
#define REMAKE_KEY_S                  83
#define REMAKE_KEY_T                  84
#define REMAKE_KEY_U                  85
#define REMAKE_KEY_V                  86
#define REMAKE_KEY_W                  87
#define REMAKE_KEY_X                  88
#define REMAKE_KEY_Y                  89
#define REMAKE_KEY_Z                  90
#define REMAKE_KEY_LEFT_BRACKET       91  /* [ */
#define REMAKE_KEY_BACKSLASH          92  /* \ */
#define REMAKE_KEY_RIGHT_BRACKET      93  /* ] */
#define REMAKE_KEY_GRAVE_ACCENT       96  /* ` */
#define REMAKE_KEY_WORLD_1            161 /* non-US #1 */
#define REMAKE_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define REMAKE_KEY_ESCAPE             256
#define REMAKE_KEY_ENTER              257
#define REMAKE_KEY_TAB                258
#define REMAKE_KEY_BACKSPACE          259
#define REMAKE_KEY_INSERT             260
#define REMAKE_KEY_DELETE             261
#define REMAKE_KEY_RIGHT              262
#define REMAKE_KEY_LEFT               263
#define REMAKE_KEY_DOWN               264
#define REMAKE_KEY_UP                 265
#define REMAKE_KEY_PAGE_UP            266
#define REMAKE_KEY_PAGE_DOWN          267
#define REMAKE_KEY_HOME               268
#define REMAKE_KEY_END                269
#define REMAKE_KEY_CAPS_LOCK          280
#define REMAKE_KEY_SCROLL_LOCK        281
#define REMAKE_KEY_NUM_LOCK           282
#define REMAKE_KEY_PRINT_SCREEN       283
#define REMAKE_KEY_PAUSE              284
#define REMAKE_KEY_F1                 290
#define REMAKE_KEY_F2                 291
#define REMAKE_KEY_F3                 292
#define REMAKE_KEY_F4                 293
#define REMAKE_KEY_F5                 294
#define REMAKE_KEY_F6                 295
#define REMAKE_KEY_F7                 296
#define REMAKE_KEY_F8                 297
#define REMAKE_KEY_F9                 298
#define REMAKE_KEY_F10                299
#define REMAKE_KEY_F11                300
#define REMAKE_KEY_F12                301
#define REMAKE_KEY_F13                302
#define REMAKE_KEY_F14                303
#define REMAKE_KEY_F15                304
#define REMAKE_KEY_F16                305
#define REMAKE_KEY_F17                306
#define REMAKE_KEY_F18                307
#define REMAKE_KEY_F19                308
#define REMAKE_KEY_F20                309
#define REMAKE_KEY_F21                310
#define REMAKE_KEY_F22                311
#define REMAKE_KEY_F23                312
#define REMAKE_KEY_F24                313
#define REMAKE_KEY_F25                314
#define REMAKE_KEY_KP_0               320
#define REMAKE_KEY_KP_1               321
#define REMAKE_KEY_KP_2               322
#define REMAKE_KEY_KP_3               323
#define REMAKE_KEY_KP_4               324
#define REMAKE_KEY_KP_5               325
#define REMAKE_KEY_KP_6               326
#define REMAKE_KEY_KP_7               327
#define REMAKE_KEY_KP_8               328
#define REMAKE_KEY_KP_9               329
#define REMAKE_KEY_KP_DECIMAL         330
#define REMAKE_KEY_KP_DIVIDE          331
#define REMAKE_KEY_KP_MULTIPLY        332
#define REMAKE_KEY_KP_SUBTRACT        333
#define REMAKE_KEY_KP_ADD             334
#define REMAKE_KEY_KP_ENTER           335
#define REMAKE_KEY_KP_EQUAL           336
#define REMAKE_KEY_LEFT_SHIFT         340
#define REMAKE_KEY_LEFT_CONTROL       341
#define REMAKE_KEY_LEFT_ALT           342
#define REMAKE_KEY_LEFT_SUPER         343
#define REMAKE_KEY_RIGHT_SHIFT        344
#define REMAKE_KEY_RIGHT_CONTROL      345
#define REMAKE_KEY_RIGHT_ALT          346
#define REMAKE_KEY_RIGHT_SUPER        347
#define REMAKE_KEY_MENU               348

