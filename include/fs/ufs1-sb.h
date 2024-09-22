/*
 * UFS1 Super Block
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef FS_UFS1_SB_H
#define FS_UFS1_SB_H  1

#include <stdint.h>

#include <fs/ufs1-stat.h>

struct ufs1_sb {
	int		dev;
	uint32_t	sblkno, cblkno, iblkno, dblkno;
	int32_t		cgoffset, cgmask;
	uint32_t	ncg, bshift, fshift, inopb;
	uint32_t	cgsize, ipg, fpg;
	struct ufs1_cs	stat;
};

int  ufs1_sb_init (struct ufs1_sb *o, int dev);
void ufs1_sb_fini (struct ufs1_sb *o);

/*
 * Next set of functions returns fragment number
 */
static inline int32_t ufs1_cg_start (const struct ufs1_sb *o, uint32_t cgx)
{
	return o->fpg * cgx + o->cgoffset * (cgx & ~o->cgmask);
}

static inline int32_t ufs1_cg_sblkno (const struct ufs1_sb *o, uint32_t cgx)
{
	return ufs1_cg_start (o, cgx) + o->sblkno;
}

static inline int32_t ufs1_cg_cblkno (const struct ufs1_sb *o, uint32_t cgx)
{
	return ufs1_cg_start (o, cgx) + o->cblkno;
}

static inline int32_t ufs1_cg_iblkno (const struct ufs1_sb *o, uint32_t cgx)
{
	return ufs1_cg_start (o, cgx) + o->iblkno;
}

static inline int32_t ufs1_cg_dblkno (const struct ufs1_sb *o, uint32_t cgx)
{
	return ufs1_cg_start (o, cgx) + o->dblkno;
}

#endif  /* FS_UFS1_SB_H */
