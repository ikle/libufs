/*
 * Marten Generic Atomic Variable
 *
 * Copyright (c) 2019-2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_ATOMIC_H
#define MARTEN_ATOMIC_H  1

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L && !defined (__STDC_NO_ATOMICS__)

#include <stdatomic.h>

#define ATOMIC_INIT	0
#define atomic_t	atomic_long

#endif  /* have C11 atomics */
#endif  /* __STDC_VERSION__ */

#ifndef ATOMIC_INIT
#error "Unsupported platform"
#endif

#endif  /* MARTEN_ATOMIC_H */
