/*
 * Marten Device Block
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_DEVICE_BLOCK_H
#define MARTEN_DEVICE_BLOCK_H  1

#include <stddef.h>
#include <sys/types.h>

void *dev_block_get (int dev, off_t offset, size_t count, int pull);
void  dev_block_put (void *o, size_t count);

#endif  /* MARTEN_DEVICE_BLOCK_H */
