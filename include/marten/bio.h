/*
 * Block Device I/O
 *
 * Copyright (c) 2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_BIO_H
#define MARTEN_BIO_H  1

#include <marten/aio.h>
#include <marten/atomic.h>
#include <marten/bool.h>
#include <marten/rwlock.h>

#define BIO_R		1
#define BIO_W		2
#define BIO_RW		(BIO_R | BIO_W)

#define BIO_READY	(1 << 0)	/* actual data available	*/
#define BIO_DIRTY	(1 << 1)	/* data modified in-core	*/

struct bio {
	rwlock_t	bio_lock;
	atomic_t	bio_ref;
	int		bio_state;
	struct aio	bio_cb;
};

#define bio_dev		bio_cb.aio_fildes
#define bio_data	bio_cb.aio_buf
#define bio_count	bio_cb.aio_nbytes
#define bio_offset	bio_cb.aio_offset

/*
 * Low-Level API
 */

static inline bool bio_load_emit (struct bio *o)
{
	return aio_read (&o->bio_cb) == 0;
}

static inline bool bio_save_emit (struct bio *o)
{
	return aio_write (&o->bio_cb) == 0;
}

static inline bool bio_join (struct bio *o)
{
	return aio_join (&o->bio_cb) == o->bio_count;
}

static inline struct bio *bio_ref (struct bio *o)
{
	atomic_fetch_add_explicit (&o->bio_ref, 1, memory_order_relaxed);
	return o;
}

struct bio *bio_get (int dev, off_t offset, size_t count, int mode);

bool bio_load (struct bio *o);
bool bio_save (struct bio *o);

static inline bool bio_load_async (struct bio *o)
{
	return (o->bio_state & BIO_READY) != 0 || bio_load_emit (o);
}

static inline bool bio_save_async (struct bio *o)
{
	return (o->bio_state & BIO_DIRTY) == 0 || bio_save_emit (o);
}

/*
 * High-Level API
 */

struct bio *bio_read  (int dev, off_t offset, size_t count);
struct bio *bio_write (int dev, off_t offset, size_t count, bool modify);
void bio_put  (struct bio *o);
bool bio_sync (struct bio *o);
void bio_read_ahead (int dev, off_t offset, size_t count);

static inline bool bio_read_begin (struct bio *o)
{
	rwlock_rdlock (&o->bio_lock);

	if (bio_load (o))
		return true;

	rwlock_unlock (&o->bio_lock);
	return false;
}

static inline bool bio_write_begin (struct bio *o, bool modify)
{
	rwlock_wrlock (&o->bio_lock);

	if (!modify || bio_load (o))
		return true;

	rwlock_unlock (&o->bio_lock);
	return false;
}

static inline bool bio_read_end (struct bio *o)
{
	rwlock_unlock (&o->bio_lock);
	return true;
}

static inline bool bio_write_end (struct bio *o, bool dirty)
{
	bool ok = true;

	if (dirty) {
		o->bio_state |= (BIO_READY | BIO_DIRTY);
		ok = bio_save_emit (o);
	}

	rwlock_unlock (&o->bio_lock);
	return ok;
}

#endif  /* MARTEN_BIO_H */
