/*
 * UFS1 On-Disk Directory Entry, Format Version 2
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef FS_UFS1_DIRENT_V2_H
#define FS_UFS1_DIRENT_v2_H  1

#include <stddef.h>
#include <stdint.h>

#define UFS1_DFSIZE	512	/* directory fragment size */

/*
 * Introduced by 4.4BSD
 */
struct ufs1_dirent {
	int32_t		d_ino;		/* dirhash uses negative marks	*/
	uint16_t	d_reclen;
	uint8_t		d_type;
	uint8_t		d_namlen;
	uint8_t		d_name[256];
};

/*
 * These constants are first defined in 4.4BSD and have the same values in
 * FreeBSD, illumos, Linux, NetBSD and OpenBSD. The DT_WHT type introduced
 * by FreeBSD 3.0.
 */
#ifndef DT_UNKNOWN

#define DT_UNKNOWN	0	/* use i_mode to detect file type	*/
#define DT_FIFO		1	/* FIFO					*/
#define DT_CHR		2	/* character device			*/
#define DT_DIR		4	/* directory				*/
#define DT_BLK		6	/* block device				*/
#define DT_REG		8	/* regular file				*/
#define DT_LNK		10	/* symbolic link			*/
#define DT_SOCK		12	/* UNIX socket				*/
#define DT_WHT		14	/* whiteout file, used for unionfs	*/

/* Convert file mode to directory entry type and vice versa */

#define IFTODT(mode)	(((mode) & 0170000) >> 12)
#define DTTOIF(type)	((type) << 12)

#endif  /* DT_UNKNOWN */

static inline int ufs1_dirent_valid (const struct ufs1_dirent *o, size_t space)
{
	const int32_t core = offsetof (struct ufs1_dirent, d_name);

	return space >= core && (o->d_reclen & 3) == 0 &&
	       (o->d_reclen - core) >= o->d_namlen;
}

static inline struct ufs1_dirent *ufs1_dirent_next (const struct ufs1_dirent *o)
{
	return (void *) o + o->d_reclen;
}

#endif  /* FS_UFS1_DIRENT_V2_H */
