/* [=]===^=====================================================================================^===[=]
 *     A Better behaved random number generator, this is slow, not to use in remakes!
 */
uint32_t mks_rand(int max) {
	uint32_t r;
#ifdef _WIN32
	HCRYPTPROV hCryptProv;
	if(CryptAcquireContext(&hCryptProv, 0, "Microsoft Base Cryptographic Provider v1.0", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		if(!CryptGenRandom(hCryptProv, sizeof(r), (BYTE*)&r)) {
			fprintf(stderr, "Error generating random number on Windows\n");
			exit(EXIT_FAILURE);
		}
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
