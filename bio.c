/*
 * Block Device I/O
 *
 * Copyright (c) 2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <marten/bio-cache.h>

static struct bio *bio_alloc (int dev, off_t offset, size_t count, int mode)
{
	struct bio *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if ((o->bio_data = malloc (count)) == NULL)
		goto no_data;

	rwlock_init (&o->bio_lock);
	memset (&o->bio_cb, 0, sizeof (o->bio_cb));

	o->bio_ref    = 2;	/* one for retval, plus one for cache	*/
	o->bio_state  = 0;
	o->bio_dev    = dev;
	o->bio_count  = count;
	o->bio_offset = offset;

	if ((mode & BIO_R) != 0 && !bio_load_emit (o))
		goto no_read;

	bio_cache_push (o);
	return o;
no_read:
no_data:
	free (o);
	return NULL;
}

static void bio_free (struct bio *o)
{
	bio_save (o);  /* to do: bio_save_async and free in io-complete */
	free ((void *) o->bio_data);
	free (o);
}

struct bio *bio_get (int dev, off_t offset, size_t count, int mode)
{
	struct bio *o = bio_cache_lookup (dev, offset, count);

	return o != NULL ? o : bio_alloc (dev, offset, count, mode);
}

void bio_put (struct bio *o)
{
	if (atomic_fetch_sub_explicit (&o->bio_ref, 1,
				       memory_order_release) != 1)
		return;

	atomic_thread_fence (memory_order_acquire);
	bio_free (o);
}

bool bio_load (struct bio *o)
{
	if ((o->bio_state & BIO_READY) != 0)
		return true;

	if (!bio_load_emit (o) || !bio_join (o))
		return false;

	o->bio_state |= BIO_READY;
	return true;
}

bool bio_save (struct bio *o)
{
	if ((o->bio_state & BIO_DIRTY) == 0)
		return true;

	if (!bio_save_emit (o) || !bio_join (o))
		return false;

	o->bio_state &= ~BIO_DIRTY;
	return true;
}

struct bio *bio_read (int dev, off_t offset, size_t count)
{
	struct bio *o;

	if ((o = bio_get (dev, offset, count, BIO_R)) == NULL ||
	    bio_read_begin (o))
		return o;

	bio_put (o);
	return NULL;
}

struct bio *bio_write (int dev, off_t offset, size_t count, bool modify)
{
	const int mode = modify ? BIO_RW : BIO_W;
	struct bio *o;

	if ((o = bio_get (dev, offset, count, mode)) == NULL ||
	    bio_write_begin (o, modify))
		return o;

	bio_put (o);
	return NULL;
}

bool bio_sync (struct bio *o)
{
	int ok;

	rwlock_wrlock (&o->bio_lock);
	ok = bio_save (o);
	rwlock_unlock (&o->bio_lock);

	return ok;
}

void bio_read_ahead (int dev, off_t offset, size_t count)
{
	struct bio *o;

	if ((o = bio_get (dev, offset, count, BIO_R)) == NULL)
		return;

	rwlock_rdlock (&o->bio_lock);
	bio_load_async (o);
	rwlock_unlock (&o->bio_lock);

	bio_put (o);
}
