

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

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
