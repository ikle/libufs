/*
 * UNIX File System v1 On-Disk Statistics
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_FS_UFS1_STAT_H
#define SYS_FS_UFS1_STAT_H  1

#include <stdint.h>

struct ufs1_cs {
	int32_t		cs_ndir;	/* number of directories	*/
	int32_t		cs_nbfree;	/* number of free blocks	*/
	int32_t		cs_nifree;	/* number of free inodes	*/
	int32_t		cs_nffree;	/* number of free frags		*/
};

#endif  /* SYS_FS_UFS1_STAT_H */
