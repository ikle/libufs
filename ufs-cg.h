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
	return o->cg_ipg * o->cg_cgx + i;
}

#include <fs/ufs1-inode.h>
#include <marten/device/block.h>

static inline
struct ufs1_inode *ufs1_cg_inode_get (const struct ufs_cg *c, int n, int pull)
{
	const off_t  base = ufs_cg_iblkno (c->sb, c->cg_cgx);
	const size_t size = sizeof (struct ufs1_inode);
	const off_t  pos  = (base << c->sb->s_fshift) + n * size;

	return dev_block_get (c->sb->dev, pos, size, pull);
}

static inline void ufs1_cg_inode_put (struct ufs1_inode *o)
{
	dev_block_put (o, sizeof (*o));
}

#endif  /* UFS_CG_H */
