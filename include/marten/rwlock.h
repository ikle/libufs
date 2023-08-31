/*
 * Marten Read-Write Lock
 *
 * Copyright (c) 2019-2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_RWLOCK_H
#define MARTEN_RWLOCK_H  1

#ifdef __unix__
#include <unistd.h>

#ifdef _POSIX_THREADS
#include <pthread.h>

#define RWLOCK_INIT	PTHREAD_RWLOCK_INITIALIZER
#define rwlock_t	pthread_rwlock_t
#define rwlock_init(o)	pthread_rwlock_init (o, NULL)
#define rwlock_rdlock	pthread_rwlock_rdlock
#define rwlock_wrlock	pthread_rwlock_wrlock
#define rwlock_unlock	pthread_rwlock_unlock

#endif  /* _POSIX_THREADS */
#endif  /* __unix__ */

#ifndef RWLOCK_INIT
#error "Unsupported platform"
#endif

#endif  /* MARTEN_RWLOCK_H */
