/*
 * Device Block Simplified API
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DEV_BLOCK_H
#define DEV_BLOCK_H  1

#include <stddef.h>
#include <sys/types.h>

void *dev_block_get (int dev, off_t offset, size_t count, int pull);
void  dev_block_put (void *o, size_t count);

#endif  /* DEV_BLOCK_H */
