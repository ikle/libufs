/*
 * Block Device I/O Cache
 *
 * Copyright (c) 2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_BIO_CACHE_H
#define MARTEN_BIO_CACHE_H  1

#include <marten/bio.h>

struct bio *bio_cache_lookup (int dev, off_t offset, size_t count);
void bio_cache_push (struct bio *o);

#endif  /* MARTEN_BIO_CACHE_H */
