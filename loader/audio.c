#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define FRAME_SIZE (NUM_CHANNELS * sizeof(short))

static void audio_callback(void *userdata, int16_t *audio_buffer, size_t frames) {
	struct loader_state *state = userdata;
	switch(state->mode) {
		case REMAKE_STATE: {
			if(state->remake->audio_callback) {
				state->remake->audio_callback(&state->shared, audio_buffer, frames);
			}
		} break;
		case SELECTOR_STATE: {
			if(state->selector->audio_callback) {
				state->selector->audio_callback(&state->shared, audio_buffer, frames);
			}
		} break;
		case LOAD_SELECTOR_STATE:
		case LOAD_REMAKE_STATE:
		case UNLOAD_REMAKE_STATE: {
			memset(audio_buffer, 0, frames * FRAME_SIZE);
		} break;
	}
}

#ifdef __linux__

/*
 *            :::        ::::::::::: ::::    ::: :::    ::: :::    :::
 *            :+:            :+:     :+:+:   :+: :+:    :+: :+:    :+:
 *            +:+            +:+     :+:+:+  +:+ +:+    +:+  +:+  +:+
 *            +#+            +#+     +#+ +:+ +#+ +#+    +:+   +#++:+
 *            +#+            +#+     +#+  +#+#+# +#+    +#+  +#+  +#+
 *            #+#            #+#     #+#   #+#+# #+#    #+# #+#    #+#
 *            ########## ########### ###    ####  ########  ###    ###
 */

#include <pthread.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE (512 * FRAME_SIZE)

snd_pcm_t *pcm;
pthread_t audio_thread;

static int16_t alsa_buffer[BUFFER_SIZE];

static void *audio_thread_func(void *arg) {
	while (1) {
		snd_pcm_wait(pcm, -1);
		audio_callback(arg, alsa_buffer, BUFFER_SIZE / FRAME_SIZE);
		snd_pcm_writei(pcm, alsa_buffer, BUFFER_SIZE / FRAME_SIZE);
		pthread_testcancel();
	}
	return 0;
}

static void audio_initialize(struct loader_state *state) {
	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, NUM_CHANNELS, SAMPLE_RATE, 1, 20000);	// 20000 is in us, we want 20ms so 20000, not sure how low we can go before breaking things on slow machines.
	snd_pcm_start(pcm);
	pthread_create(&audio_thread, 0, audio_thread_func, state);
}

static void audio_shutdown() {
	pthread_cancel(audio_thread);
	pthread_join(audio_thread, 0);
	snd_pcm_drop(pcm);
	snd_pcm_close(pcm);
}

#elif _WIN32

/*
 *  :::       ::: ::::::::::: ::::    ::: :::::::::   ::::::::  :::       :::  ::::::::
 *  :+:       :+:     :+:     :+:+:   :+: :+:    :+: :+:    :+: :+:       :+: :+:    :+:
 *  +:+       +:+     +:+     :+:+:+  +:+ +:+    +:+ +:+    +:+ +:+       +:+ +:+
 *  +#+  +:+  +#+     +#+     +#+ +:+ +#+ +#+    +:+ +#+    +:+ +#+  +:+  +#+ +#++:++#++
 *  +#+ +#+#+ +#+     +#+     +#+  +#+#+# +#+    +#+ +#+    +#+ +#+ +#+#+ +#+        +#+
 *   #+#+# #+#+#      #+#     #+#   #+#+# #+#    #+# #+#    #+#  #+#+# #+#+#  #+#    #+#
 *    ###   ###   ########### ###    #### #########   ########    ###   ###    ########
 */


#include <windows.h>
#include <mmsystem.h>

#define BUFFER_COUNT 3
#define BUFFER_SIZE (1024 * FRAME_SIZE)

HWAVEOUT wave_out;
WAVEHDR wave_header[BUFFER_COUNT];
static int8_t waveout_buffer[BUFFER_COUNT][BUFFER_SIZE];

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	if(uMsg == WOM_DONE) {
		struct loader_state *state = (struct loader_state *)dwInstance;

		WAVEHDR *wave_header = (WAVEHDR*)dwParam1;
		waveOutUnprepareHeader(hwo, wave_header, sizeof(WAVEHDR));
		audio_callback(state, (int16_t*)wave_header->lpData, wave_header->dwBufferLength / FRAME_SIZE);
		waveOutPrepareHeader(hwo, wave_header, sizeof(WAVEHDR));
		waveOutWrite(hwo, wave_header, sizeof(WAVEHDR));
	}
}

static void audio_initialize(struct loader_state *state) {
	WAVEFORMATEX wave_format = {0};
	wave_format.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.nChannels = NUM_CHANNELS;
	wave_format.nSamplesPerSec = SAMPLE_RATE;
	wave_format.wBitsPerSample = 16;
	wave_format.nBlockAlign = wave_format.nChannels * (wave_format.wBitsPerSample / 8);
	wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
	waveOutOpen(&wave_out, WAVE_MAPPER, &wave_format, (DWORD_PTR)waveOutProc, (DWORD_PTR)state, CALLBACK_FUNCTION);

	for(uint32_t i = 0; i < BUFFER_COUNT; ++i) {
		memset(&wave_header[i], 0, sizeof(WAVEHDR));
		wave_header[i].lpData = (char*)waveout_buffer[i];
		wave_header[i].dwBufferLength = BUFFER_SIZE;
		waveOutPrepareHeader(wave_out, &wave_header[i], sizeof(WAVEHDR));
	}

	for(uint32_t i = 0; i < BUFFER_COUNT; ++i) {
		memset(wave_header[i].lpData, 0, BUFFER_SIZE);
		waveOutWrite(wave_out, &wave_header[i], sizeof(WAVEHDR));
	}
}

static void audio_shutdown() {
	waveOutReset(wave_out);
	for(uint32_t i = 0; i < BUFFER_COUNT; ++i) {
		waveOutUnprepareHeader(wave_out, &wave_header[i], sizeof(WAVEHDR));
	}
	waveOutClose(wave_out);
}

#else
	#error "Unsupported platform"
#endif
