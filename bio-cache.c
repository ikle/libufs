/*
 * Block Device I/O Cache
 *
 * Copyright (c) 2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <marten/bio-cache.h>
#include <marten/hash.h>
#include <marten/mutex.h>

#define BIO_CACHE_ORDER		10
#define BIO_CACHE_SIZE		(1UL << BIO_CACHE_ORDER)
#define BIO_CACHE_MASK		(BIO_CACHE_SIZE - 1UL)

static mutex_t cache_lock = MUTEX_INIT;
static struct bio *cache[BIO_CACHE_SIZE];

static size_t bio_cache_index (int dev, off_t offset)
{
	uint32_t iv = 0;

	iv = oat_hash_step (iv, dev);
	iv = oat_hash_step (iv, offset);
	iv = oat_hash_step (iv, offset >> 32);

	return oat_hash_final (iv) & BIO_CACHE_MASK;
}

struct bio *bio_cache_lookup (int dev, off_t offset, size_t count)
{
	const size_t i = bio_cache_index (dev, offset);
	struct bio *o, *ret = NULL;

	mutex_lock (&cache_lock);
	o = cache[i];

	if (o->bio_dev == dev && o->bio_offset == offset &&
	    o->bio_count >= count)
		ret = bio_ref (o);

	mutex_unlock (&cache_lock);
	return ret;
}

void bio_cache_push (struct bio *o)
{
	const size_t i = bio_cache_index (o->bio_dev, o->bio_offset);
	struct bio *old;

	mutex_lock (&cache_lock);
	old = cache[i];
	cache[i] = o;
	mutex_unlock (&cache_lock);

	if (old != NULL)
		bio_put (old);
}
