/*
 * Marten Generic Hash
 *
 * Copyright (c) 2010-2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_HASH_H
#define MARTEN_HASH_H  1

#include <stdint.h>

/*
 * Jenkins One-at-a-time hash single step
 */
static inline uint32_t oat_hash_step (uint32_t iv, uint32_t unit)
{
	iv += unit;
	iv += iv << 10;
	iv ^= iv >> 6;

	return iv;
}

/*
 * Jenkins One-at-a-time hash finalization
 */
static inline uint32_t oat_hash_final (uint32_t iv)
{
	iv += iv << 3;
	iv ^= iv >> 11;
	iv += iv << 15;

	return iv;
}

#endif  /* MARTEN_HASH_H */
