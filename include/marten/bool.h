/*
 * Marten Generic Boolean Type
 *
 * Copyright (c) 2019-2023 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MARTEN_BOOL_H
#define MARTEN_BOOL_H  1

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

#include <stdbool.h>

#else  /* have C99 */

#define bool	int
#define false	0
#define true	1

#endif  /* have not C99 */

#endif  /* MARTEN_BOOL_H */
