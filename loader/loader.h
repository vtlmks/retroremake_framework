#define ARRAY_SIZE(_Array) (sizeof(_Array) / sizeof(_Array[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#if defined(__GNUC__) || defined(__clang__) || defined(__TINYC__)
#define ALIGNED(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#endif

static float lerp(float v0, float v1, float t) {
	return (1.f - t) * v0 + t * v1;
}

#ifdef _WIN32
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

static const char* glsl_version = "#version 140";
static const float vertices[] = {
	 1.f,-1.f, 0.0f, 1.f, 0.f,
	 1.f, 1.f, 0.0f, 1.f, 1.f,
	-1.f, 1.f, 0.0f, 0.f, 1.f,
	-1.f,-1.f, 0.0f, 0.f, 0.f
};
static const uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };
