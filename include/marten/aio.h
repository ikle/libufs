/*
 * Marten Asynchronous I/O
 *
 * Copyright (c) 2019-2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_AIO_H
#define MARTEN_AIO_H  1

#ifdef __unix__
#include <unistd.h>

#ifdef _POSIX_ASYNCHRONOUS_IO
#include <errno.h>
#include <aio.h>

#define aio	aiocb

static inline ssize_t aio_join (const struct aio *o)
{
	while (aio_error (o) == EINPROGRESS)
		aio_suspend (&o, 1, NULL);

	return aio_return ((void *) o);
}

#endif  /* _POSIX_ASYNCHRONOUS_IO */
#endif  /* __unix__ */

#ifndef aio
#error "Unsupported platform"
#endif

#endif  /* MARTEN_AIO_H */
