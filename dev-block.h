/*
 * Device Block Simplified API
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DEV_BLOCK_H
#define DEV_BLOCK_H  1

#include <stdlib.h>
#include <unistd.h>

static void *dev_block_get (int dev, off_t offset, size_t count, int pull)
{
	void *o;

	if ((o = malloc (count)) == NULL)
		return o;

	if (pull && pread (dev, o, count, offset) != count)
		goto no_read;

	return o;
no_read:
	free (o);
	return NULL;
}

static void dev_block_put (void *o, size_t count)
{
	free (o);
}

#endif  /* DEV_BLOCK_H */
