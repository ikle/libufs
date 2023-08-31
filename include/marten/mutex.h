/*
 * Marten Mutual Exclusion Lock
 *
 * Copyright (c) 2019-2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_MUTEX_H
#define MARTEN_MUTEX_H  1

#ifdef __unix__
#include <unistd.h>

#ifdef _POSIX_THREADS
#include <pthread.h>

#define MUTEX_INIT	PTHREAD_MUTEX_INITIALIZER
#define mutex_t		pthread_mutex_t
#define mutex_init(o)	pthread_mutex_init ((o), NULL)
#define mutex_lock	pthread_mutex_lock
#define mutex_unlock	pthread_mutex_unlock

#endif  /* _POSIX_THREADS */
#endif  /* __unix__ */

#ifndef MUTEX_INIT
#error "Unsupported platform"
#endif

#endif  /* MARTEN_MUTEX_H */
