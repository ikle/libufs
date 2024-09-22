/*
 * UFS1 Index Node
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef UFS1_INODE_H
#define UFS1_INODE_H  1

#include <fs/ufs1-cg.h>
#include <fs/ufs1-inode-v2.h>
#include <marten/device/block.h>

static inline
struct ufs1_inode *ufs1_cg_inode_get (const struct ufs1_cg *c, int n, int pull)
{
	const off_t  base = ufs1_cg_iblkno (c->sb, c->cgx);
	const size_t size = sizeof (struct ufs1_inode);
	const off_t  pos  = (base << c->sb->fshift) + n * size;

	return dev_block_get (c->sb->dev, pos, size, pull);
}

static inline void ufs1_inode_put (struct ufs1_inode *o)
{
	dev_block_put (o, sizeof (*o));
}

int32_t ufs1_inode_block (const struct ufs1_sb *sb, const struct ufs1_inode *o,
			  uint64_t i);

#endif  /* UFS1_INODE_H */
