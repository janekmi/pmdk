/*
 * Copyright 2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PMEM_MEMSET_SSE2_H
#define PMEM_MEMSET_SSE2_H

#include <xmmintrin.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "libpmem.h"
#include "out.h"

static inline void
memset_small_sse2(char *dest, int c, size_t len)
{
	ASSERT(len <= 64);

	if (len > 48) {
		/* 49..64 */
		__m128i xmm = _mm_set1_epi8((char)c);

		_mm_storeu_si128((__m128i *)(dest + 0), xmm);
		_mm_storeu_si128((__m128i *)(dest + 16), xmm);
		_mm_storeu_si128((__m128i *)(dest + 32), xmm);
		_mm_storeu_si128((__m128i *)(dest + len - 16), xmm);
	} else if (len > 32) {
		/* 33..48 */
		__m128i xmm = _mm_set1_epi8((char)c);

		_mm_storeu_si128((__m128i *)(dest + 0), xmm);
		_mm_storeu_si128((__m128i *)(dest + 16), xmm);
		_mm_storeu_si128((__m128i *)(dest + len - 16), xmm);
	} else if (len > 16) {
		/* 17..32 */

		__m128i xmm = _mm_set1_epi8((char)c);

		_mm_storeu_si128((__m128i *)(dest + 0), xmm);
		_mm_storeu_si128((__m128i *)(dest + len - 16), xmm);
	} else if (len > 8) {
		/* 9..16 */
		uint64_t d;
		memset(&d, c, 8);

		*(uint64_t *)dest = d;
		*(uint64_t *)(dest + len - 8) = d;
	} else if (len > 4) {
		/* 5..8 */
		uint32_t d;
		memset(&d, c, 4);

		*(uint32_t *)dest = d;
		*(uint32_t *)(dest + len - 4) = d;
	} else if (len > 2) {
		/* 3..4 */
		uint16_t d;
		memset(&d, c, 2);

		*(uint16_t *)dest = d;
		*(uint16_t *)(dest + len - 2) = d;
	} else if (len == 2) {
		uint16_t d;
		memset(&d, c, 2);

		*(uint16_t *)dest = d;
	} else {
		*(uint8_t *)dest = (uint8_t)c;
	}
}

#endif
