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

#define AMIGA_MOUSE_BUTTON_LEFT		0
#define AMIGA_MOUSE_BUTTON_RIGHT		1
#define AMIGA_MOUSE_BUTTON_MIDDLE	2	// cant be used, as the main program is set to lock the cursor when pressing middle mouse to make mousemovement work inside demos

#define AMIGA_KEY_SPACE              32
#define AMIGA_KEY_APOSTROPHE         39  /* ' */
#define AMIGA_KEY_COMMA              44  /* , */
#define AMIGA_KEY_MINUS              45  /* - */
#define AMIGA_KEY_PERIOD             46  /* . */
#define AMIGA_KEY_SLASH              47  /* / */
#define AMIGA_KEY_0                  48
#define AMIGA_KEY_1                  49
#define AMIGA_KEY_2                  50
#define AMIGA_KEY_3                  51
#define AMIGA_KEY_4                  52
#define AMIGA_KEY_5                  53
#define AMIGA_KEY_6                  54
#define AMIGA_KEY_7                  55
#define AMIGA_KEY_8                  56
#define AMIGA_KEY_9                  57
#define AMIGA_KEY_SEMICOLON          59  /* ; */
#define AMIGA_KEY_EQUAL              61  /* = */
#define AMIGA_KEY_A                  65
#define AMIGA_KEY_B                  66
#define AMIGA_KEY_C                  67
#define AMIGA_KEY_D                  68
#define AMIGA_KEY_E                  69
#define AMIGA_KEY_F                  70
#define AMIGA_KEY_G                  71
#define AMIGA_KEY_H                  72
#define AMIGA_KEY_I                  73
#define AMIGA_KEY_J                  74
#define AMIGA_KEY_K                  75
#define AMIGA_KEY_L                  76
#define AMIGA_KEY_M                  77
#define AMIGA_KEY_N                  78
#define AMIGA_KEY_O                  79
#define AMIGA_KEY_P                  80
#define AMIGA_KEY_Q                  81
#define AMIGA_KEY_R                  82
#define AMIGA_KEY_S                  83
#define AMIGA_KEY_T                  84
#define AMIGA_KEY_U                  85
#define AMIGA_KEY_V                  86
#define AMIGA_KEY_W                  87
#define AMIGA_KEY_X                  88
#define AMIGA_KEY_Y                  89
#define AMIGA_KEY_Z                  90
#define AMIGA_KEY_LEFT_BRACKET       91  /* [ */
#define AMIGA_KEY_BACKSLASH          92  /* \ */
#define AMIGA_KEY_RIGHT_BRACKET      93  /* ] */
#define AMIGA_KEY_GRAVE_ACCENT       96  /* ` */
#define AMIGA_KEY_WORLD_1            161 /* non-US #1 */
#define AMIGA_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define AMIGA_KEY_ESCAPE             256
#define AMIGA_KEY_ENTER              257
#define AMIGA_KEY_TAB                258
#define AMIGA_KEY_BACKSPACE          259
#define AMIGA_KEY_INSERT             260
#define AMIGA_KEY_DELETE             261
#define AMIGA_KEY_RIGHT              262
#define AMIGA_KEY_LEFT               263
#define AMIGA_KEY_DOWN               264
#define AMIGA_KEY_UP                 265
#define AMIGA_KEY_PAGE_UP            266
#define AMIGA_KEY_PAGE_DOWN          267
#define AMIGA_KEY_HOME               268
#define AMIGA_KEY_END                269
#define AMIGA_KEY_CAPS_LOCK          280
#define AMIGA_KEY_SCROLL_LOCK        281
#define AMIGA_KEY_NUM_LOCK           282
#define AMIGA_KEY_PRINT_SCREEN       283
#define AMIGA_KEY_PAUSE              284
#define AMIGA_KEY_F1                 290
#define AMIGA_KEY_F2                 291
#define AMIGA_KEY_F3                 292
#define AMIGA_KEY_F4                 293
#define AMIGA_KEY_F5                 294
#define AMIGA_KEY_F6                 295
#define AMIGA_KEY_F7                 296
#define AMIGA_KEY_F8                 297
#define AMIGA_KEY_F9                 298
#define AMIGA_KEY_F10                299
#define AMIGA_KEY_F11                300
#define AMIGA_KEY_F12                301
#define AMIGA_KEY_F13                302
#define AMIGA_KEY_F14                303
#define AMIGA_KEY_F15                304
#define AMIGA_KEY_F16                305
#define AMIGA_KEY_F17                306
#define AMIGA_KEY_F18                307
#define AMIGA_KEY_F19                308
#define AMIGA_KEY_F20                309
#define AMIGA_KEY_F21                310
#define AMIGA_KEY_F22                311
#define AMIGA_KEY_F23                312
#define AMIGA_KEY_F24                313
#define AMIGA_KEY_F25                314
#define AMIGA_KEY_KP_0               320
#define AMIGA_KEY_KP_1               321
#define AMIGA_KEY_KP_2               322
#define AMIGA_KEY_KP_3               323
#define AMIGA_KEY_KP_4               324
#define AMIGA_KEY_KP_5               325
#define AMIGA_KEY_KP_6               326
#define AMIGA_KEY_KP_7               327
#define AMIGA_KEY_KP_8               328
#define AMIGA_KEY_KP_9               329
#define AMIGA_KEY_KP_DECIMAL         330
#define AMIGA_KEY_KP_DIVIDE          331
#define AMIGA_KEY_KP_MULTIPLY        332
#define AMIGA_KEY_KP_SUBTRACT        333
#define AMIGA_KEY_KP_ADD             334
#define AMIGA_KEY_KP_ENTER           335
#define AMIGA_KEY_KP_EQUAL           336
#define AMIGA_KEY_LEFT_SHIFT         340
#define AMIGA_KEY_LEFT_CONTROL       341
#define AMIGA_KEY_LEFT_ALT           342
#define AMIGA_KEY_LEFT_SUPER         343
#define AMIGA_KEY_RIGHT_SHIFT        344
#define AMIGA_KEY_RIGHT_CONTROL      345
#define AMIGA_KEY_RIGHT_ALT          346
#define AMIGA_KEY_RIGHT_SUPER        347
#define AMIGA_KEY_MENU               348

