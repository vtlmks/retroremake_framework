#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define FRAME_SIZE (NUM_CHANNELS * sizeof(short))

void audio_callback(int16_t *audio_buffer, size_t frames) {
	// TODO(peter): This should call the selector/remake audio_callback() function...
	//              I have to decide if it should be called with just buffer + frames
	//              of if I should have some kind of user_data field, or if I should
	//              send a pointer to the part_state structure...  have to decide soon...

	// pt2play_FillAudioBuffer(&ptstate, audio_buffer, frames);
}

#ifdef __linux__
#include <pthread.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE (512 * FRAME_SIZE)

snd_pcm_t *pcm;
pthread_t audio_thread;

uint16_t alsa_buffer[BUFFER_SIZE];

void *audio_thread_func(void *arg) {
	while (1) {
		snd_pcm_wait(pcm, -1);
		audio_callback(alsa_buffer, BUFFER_SIZE / FRAME_SIZE);
		snd_pcm_writei(pcm, alsa_buffer, BUFFER_SIZE / FRAME_SIZE);
		pthread_testcancel();
	}

	return 0;
}

static void audio_initialize() {
	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, NUM_CHANNELS, SAMPLE_RATE, 1, 100000);
	snd_pcm_start(pcm);
	pthread_create(&audio_thread, 0, audio_thread_func, 0);
}

static void audio_shutdown() {
	pthread_cancel(audio_thread);
	pthread_join(audio_thread, 0);
	snd_pcm_drop(pcm);
	snd_pcm_close(pcm);
}

#elif _WIN32

#include <windows.h>
#include <mmsystem.h>

#define BUFFER_COUNT 3
#define BUFFER_SIZE (1024 * FRAME_SIZE)

HWAVEOUT wave_out;
WAVEHDR wave_header[BUFFER_COUNT];
int8_t waveout_buffer[BUFFER_COUNT][BUFFER_SIZE];

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	if(uMsg == WOM_DONE) {
		WAVEHDR *wave_header = (WAVEHDR*)dwParam1;
		waveOutUnprepareHeader(hwo, wave_header, sizeof(WAVEHDR));
		audio_callback((int16_t*)wave_header->lpData, wave_header->dwBufferLength / FRAME_SIZE);
		waveOutPrepareHeader(hwo, wave_header, sizeof(WAVEHDR));
		waveOutWrite(hwo, wave_header, sizeof(WAVEHDR));
	}
}

static void audio_initialize() {
	WAVEFORMATEX wave_format;
	wave_format.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.nChannels = NUM_CHANNELS;
	wave_format.nSamplesPerSec = SAMPLE_RATE;
	wave_format.wBitsPerSample = 16;
	wave_format.nBlockAlign = wave_format.nChannels * (wave_format.wBitsPerSample / 8);
	wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
	wave_format.cbSize = 0;
	waveOutOpen(&wave_out, WAVE_MAPPER, &wave_format, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);

	for(uint32_t i = 0; i < BUFFER_COUNT; ++i) {
		wave_header[i].lpData = waveout_buffer[i];
		wave_header[i].dwBufferLength = BUFFER_SIZE;
		wave_header[i].dwBytesRecorded = 0;
		wave_header[i].dwUser = 0;
		wave_header[i].dwFlags = 0;
		wave_header[i].dwLoops = 0;
		wave_header[i].lpNext = 0;
		wave_header[i].reserved = 0;
		waveOutPrepareHeader(wave_out, &wave_header[i], sizeof(WAVEHDR));
	}

	for(uint32_t i = 0; i < BUFFER_COUNT; ++i) {
		audio_callback((int16_t*)wave_header[i].lpData, BUFFER_SIZE/FRAME_SIZE);
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
