/*
 * UFS Cylinder Group
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef UFS_CG_H
#define UFS_CG_H  1

#include "ufs-sb.h"

struct ufs_cg {
	const struct ufs_sb *sb;

	void		*cg_data;
	int32_t		cg_start;
	uint32_t	cg_cgx, cg_ipg, cg_fpg;
	uint32_t	cg_imap_pos, cg_fmap_pos, cg_emap_pos;
	struct ufs1_cs	cg_stat;
};

int  ufs_cg_init (struct ufs_cg *o, const struct ufs_sb *s, uint32_t cgx);
void ufs_cg_fini (struct ufs_cg *o);

static inline uint8_t *ufs_cg_imap (const struct ufs_cg *o)
{
	return o->cg_data + o->cg_imap_pos;
}

static inline uint8_t *ufs_cg_fmap (const struct ufs_cg *o)
{
	return o->cg_data + o->cg_fmap_pos;
}

static inline uint32_t ufs_cg_ino (const struct ufs_cg *o, uint32_t i)
{
	return o->sb->s_ipg * o->cg_cgx + i;
}

#include <unistd.h>

#include <fs/ufs1-inode.h>

static inline struct ufs1_inode *
ufs1_cg_inode_pull (const struct ufs_cg *c, int n, struct ufs1_inode *buf)
{
	const off_t base = ufs_cg_iblkno (c->sb, c->cg_cgx);
	const off_t pos  = (base << c->sb->s_fshift) + n * sizeof (*buf);

	if (pread (c->sb->fd, buf, sizeof (*buf), pos) != sizeof (*buf))
		return NULL;

	return buf;
}

#endif  /* UFS_CG_H */
