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
	int		dev;
	uint32_t	sblkno, cblkno, iblkno, dblkno;
	int32_t		cgoffset, cgmask;
	uint32_t	ncg, bshift, fshift, inopb;
	uint32_t	cgsize, ipg, fpg;
	struct ufs1_cs	stat;
};

static inline void ufs_sb_fini (struct ufs_sb *o)
{
	close (o->dev);
}

static inline int ufs_sb_error (struct ufs_sb *o, const char *reason)
{
	ufs_sb_fini (o);
	return 0;
}

static inline int ufs_sb_init (struct ufs_sb *o, int dev)
{
	struct ufs1_sb_v2 buf, *s = &buf;

	if (pread (o->dev = dev, &buf, sizeof (buf), 8192) != sizeof (buf))
		return ufs_sb_error (o, "Cannot read super block");

	if (s->s_magic != UFS1_SB_MAGIC)
		return ufs_sb_error (o, "Cannot find valid super block magic");

	o->sblkno   = s->s_sblkno;
	o->cblkno   = s->s_cblkno;
	o->iblkno   = s->s_iblkno;
	o->dblkno   = s->s_dblkno;
	o->cgsize   = s->s_cgsize;
	o->cgoffset = s->s_cgoffset;
	o->cgmask   = s->s_cgmask;
	o->ncg      = s->s_ncg;
	o->ipg      = s->s_ipg;
	o->fpg      = s->s_fpg;

	if (o->sblkno >= o->cblkno || o->cblkno >= o->iblkno ||
	    o->iblkno >= o->dblkno || o->dblkno >= o->fpg ||
	    o->cgsize < sizeof (struct ufs1_cg_v2) ||
	    o->cgsize > (o->iblkno - o->cblkno) << s->s_fshift)
		return ufs_sb_error (o, "Invalid file system layout");

	o->bshift = s->s_bshift;
	o->fshift = s->s_fshift;
	o->inopb  = s->s_inopb;

	if (s->s_bshift < 12 || s->s_bsize != (1L << o->bshift) ||
	    s->s_fshift < 9  || s->s_fsize != (1L << o->fshift) ||
	    s->s_fragshift != (o->bshift - o->fshift) ||
	    s->s_fragshift < 0 || s->s_fragshift > 3 ||
	    s->s_fsbtodb != (o->fshift - 9) ||
	    s->s_frag != (1L << (o->bshift - o->fshift)) ||
	    s->s_bmask != (~0L << o->bshift) ||
	    s->s_fmask != (~0L << o->fshift) ||
	    o->inopb != (s->s_bsize / 128))
		return ufs_sb_error (o, "Invalid file system configuration");

	if (s->s_maxembedded != 60 || s->s_inodefmt != 2)
		return ufs_sb_error (o, "Unknown i-node format");

	o->stat = s->s_cstotal;
	return 1;
}

/*
 * Next set of functions returns fragment number
 */
static inline int32_t ufs_cg_start (const struct ufs_sb *o, uint32_t cgx)
{
	return o->fpg * cgx + o->cgoffset * (cgx & ~o->cgmask);
}

static inline int32_t ufs_cg_sblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->sblkno;
}

static inline int32_t ufs_cg_cblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->cblkno;
}

static inline int32_t ufs_cg_iblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->iblkno;
}

static inline int32_t ufs_cg_dblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->dblkno;
}

#endif  /* UFS_SB_H */
