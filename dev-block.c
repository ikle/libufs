/*
 * Device Block Simplified API
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <unistd.h>

#include "dev-block.h"

void *dev_block_get (int dev, off_t offset, size_t count, int pull)
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

void dev_block_put (void *o, size_t count)
{
	free (o);
}
