#pragma once

struct rng_state { uint32_t x, y, z, w; };

// A Better behaved random number generator, this is slow, not to use in remakes!
uint32_t mks_rand(int max);

// Xor-rng good enough random numbers very fast...
void xor_init_rng(struct rng_state *state, uint32_t seed);
uint32_t xor_generate_random(struct rng_state *state);


#ifdef UTILS_IMPLEMENTATION

void xor_init_rng(struct rng_state *state, uint32_t seed) {
	state->x = seed;
	state->y = seed ^ 362436069;
	state->z = seed ^ 521288629;
	state->w = seed ^ 88675123;
}

uint32_t xor_generate_random(struct rng_state *state) {
	uint32_t t = state->x ^ (state->x << 11);
	state->x = state->y;
	state->y = state->z;
	state->z = state->w;
	state->w = (state->w ^ (state->w >> 19)) ^ (t ^ (t >> 8));
	return state->w;
}

/*
 *     A Better behaved random number generator, this is slow, not to use in remakes!
 */
uint32_t mks_rand(int max) {
	uint32_t r;
#ifdef _WIN32
	HCRYPTPROV hCryptProv;
	if (CryptAcquireContext(&hCryptProv, 0, "Microsoft Base Cryptographic Provider v1.0", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		if (!CryptGenRandom(hCryptProv, sizeof(r), (BYTE*)&r)) {
			fprintf(stderr, "Error generating random number on Windows\n");
			exit(EXIT_FAILURE);
		}
		// Always release the context after use
		if (!CryptReleaseContext(hCryptProv, 0)) {
			fprintf(stderr, "Failed to release context: %d\n", GetLastError());
		}
	} else {
		fprintf(stderr, "Failed to acquire cryptographic context: %d\n", GetLastError());
		exit(EXIT_FAILURE);
	}
#else
	int fd = open("/dev/urandom", O_RDONLY);
	if(fd == -1) {
		perror("Error opening /dev/urandom");
		exit(EXIT_FAILURE);
	}
	if(read(fd, &r, sizeof(r)) != sizeof(r)) {
		perror("Error reading from /dev/urandom");
		exit(EXIT_FAILURE);
	}
	close(fd);
#endif
	return r % max;
}

#endif
