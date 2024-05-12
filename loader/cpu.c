#pragma once

#include <cpuid.h>

static int check_xgetbv_ymm() {
	unsigned eax, edx;
	__asm__ __volatile__(
		"xgetbv" : "=a"(eax), "=d"(edx) : "c"(0)
	);
	return (eax & 0x6) == 0x6; // Check if YMM registers (bit 1) and XMM registers (bit 2) are preserved
}

static void check_cpu_features(struct loader_state *state) {
	unsigned eax = 0;
	unsigned ebx = 0;
	unsigned ecx = 0;
	unsigned edx = 0;

	uint32_t cpu_features = 0;

	// Basic CPUID Information
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	if (edx & (1 << 25)) cpu_features |= REMAKE_CPU_SSE;     // SSE
	if (edx & (1 << 26)) cpu_features |= REMAKE_CPU_SSE2;    // SSE2
	if (ecx & (1 << 0)) cpu_features |= REMAKE_CPU_SSE3;     // SSE3
	if (ecx & (1 << 9)) cpu_features |= REMAKE_CPU_SSSE3;    // SSSE3
	if (ecx & (1 << 19)) cpu_features |= REMAKE_CPU_SSE4_1;  // SSE4.1
	if (ecx & (1 << 20)) cpu_features |= REMAKE_CPU_SSE4_2;  // SSE4.2
	if (ecx & (1 << 28) && (ecx & (1 << 27)) && check_xgetbv_ymm()) cpu_features |= REMAKE_CPU_AVX;     // AVX, checking OSXSAVE and XSAVE states
	if (ecx & (1 << 25)) cpu_features |= REMAKE_CPU_AES;     // AES-NI

	// Extended Features (CPUID with EAX=7, ECX=0)
	__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
	if ((ebx & (1 << 5)) && (ecx & (1 << 27)) && check_xgetbv_ymm()) cpu_features |= REMAKE_CPU_AVX2; // AVX2, checking OSXSAVE and XSAVE states
	if (ebx & (1 << 3)) cpu_features |= REMAKE_CPU_BMI1;      // BMI1
	if (ebx & (1 << 8)) cpu_features |= REMAKE_CPU_BMI2;      // BMI2
	if (ebx & (1 << 12)) cpu_features |= REMAKE_CPU_SHA;      // SHA Extensions
	if (ebx & (1 << 16)) cpu_features |= REMAKE_CPU_AVX512F;  // AVX-512 Foundation

	// Check for FMA3
	if (ecx & (1 << 12)) cpu_features |= REMAKE_CPU_FMA;      // FMA3

	state->shared.cpu_features = cpu_features;
}
