/*
 * UFS Super Block
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef UFS_SB_H
#define UFS_SB_H  1

#include <unistd.h>

#include <fs/ufs1-cg.h>

struct ufs_sb {
	int		fd;
	uint32_t	s_sblkno, s_cblkno, s_iblkno, s_dblkno;
	int32_t		s_cgoffset, s_cgmask;
	uint32_t	s_ncg, s_bshift, s_fshift, s_inopb;
	uint32_t	s_cgsize, s_ipg, s_fpg;
	struct ufs1_cs	s_stat;
};

static void ufs_sb_fini (struct ufs_sb *o)
{
	close (o->fd);
}

static int ufs_sb_error (struct ufs_sb *o, const char *reason)
{
	ufs_sb_fini (o);
	return 0;
}

static int ufs_sb_init (struct ufs_sb *o, int fd)
{
	struct ufs1_sb buf, *s = &buf;

	if (pread (o->fd = fd, &buf, sizeof (buf), 8192) != sizeof (buf))
		return ufs_sb_error (o, "Cannot read super block");

	if (s->s_magic != UFS1_SB_MAGIC)
		return ufs_sb_error (o, "Cannot find valid super block magic");

	o->s_sblkno   = s->s_sblkno;
	o->s_cblkno   = s->s_cblkno;
	o->s_iblkno   = s->s_iblkno;
	o->s_dblkno   = s->s_dblkno;
	o->s_cgsize   = s->s_cgsize;
	o->s_cgoffset = s->s_cgoffset;
	o->s_cgmask   = s->s_cgmask;
	o->s_ncg      = s->s_ncg;
	o->s_ipg      = s->s_ipg;
	o->s_fpg      = s->s_fpg;

	if (o->s_sblkno >= o->s_cblkno || o->s_cblkno >= o->s_iblkno ||
	    o->s_iblkno >= o->s_dblkno || o->s_dblkno >= o->s_fpg ||
	    o->s_cgsize < sizeof (struct ufs1_cg) ||
	    o->s_cgsize > (o->s_iblkno - o->s_cblkno) << s->s_fshift)
		return ufs_sb_error (o, "Invalid file system layout");

	o->s_bshift = s->s_bshift;
	o->s_fshift = s->s_fshift;
	o->s_inopb  = s->s_inopb;

	if (s->s_bshift < 12 || s->s_bsize != (1L << o->s_bshift) ||
	    s->s_fshift < 9  || s->s_fsize != (1L << o->s_fshift) ||
	    s->s_fragshift != (o->s_bshift - o->s_fshift) ||
	    s->s_fragshift < 0 || s->s_fragshift > 3 ||
	    s->s_fsbtodb != (o->s_fshift - 9) ||
	    s->s_frag != (1L << (o->s_bshift - o->s_fshift)) ||
	    s->s_bmask != (~0L << o->s_bshift) ||
	    s->s_fmask != (~0L << o->s_fshift) ||
	    o->s_inopb != (s->s_bsize / 128))
		return ufs_sb_error (o, "Invalid file system configuration");

	if (s->s_maxembedded != 60 || s->s_inodefmt != 2)
		return ufs_sb_error (o, "Unknown i-node format");

	o->s_stat = s->s_cstotal;
	return 1;
}

static inline int32_t ufs_cg_start (const struct ufs_sb *o, uint32_t cgx)
{
	return o->s_fpg * cgx + o->s_cgoffset * (cgx & ~o->s_cgmask);
}

static inline int32_t ufs_cg_cblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->s_cblkno;
}

static inline int32_t ufs_cg_iblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->s_iblkno;
}

static inline int32_t ufs_cg_dblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->s_dblkno;
}

#endif  /* UFS_SB_H */
