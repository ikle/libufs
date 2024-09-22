/*
 * UFS1 Cylinder Group
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef FS_UFS1_CG_H
#define FS_UFS1_CG_H  1

#include <fs/ufs1-sb.h>

struct ufs1_cg {
	struct ufs1_sb *sb;

	void		*data;
	int32_t		start;
	uint32_t	cgx, ipg, fpg;
	uint32_t	imap_pos, fmap_pos, emap_pos;
	struct ufs1_cs	stat;
};

int  ufs1_cg_init (struct ufs1_cg *o, struct ufs1_sb *s, uint32_t cgx);
void ufs1_cg_fini (struct ufs1_cg *o);

static inline uint8_t *ufs1_cg_imap (const struct ufs1_cg *o)
{
	return o->data + o->imap_pos;		/* [(ipg + 7) / 8] */
}

static inline uint8_t *ufs1_cg_fmap (const struct ufs1_cg *o)
{
	return o->data + o->fmap_pos;		/* [(fpg + 7) / 8] */
}

static inline uint32_t ufs1_cg_ino (const struct ufs1_cg *o, uint32_t i)
{
	return o->ipg * o->cgx + i;
}

#endif  /* FS_UFS1_CG_H */
