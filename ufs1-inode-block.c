/*
 * UFS1 Inode Block Search
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ufs1-inode.h"

static int32_t
ufs1_block_map (const struct ufs1_sb *sb, const struct ufs1_inode *o,
		uint64_t at, unsigned order, size_t i)
{
	const off_t  pos   = (off_t) at << sb->fshift;
	const size_t bsize = (size_t) 4 << order;
	int32_t *db, frag;

	if ((db = dev_block_get (sb->dev, pos, bsize, 1)) == NULL)
		return -1;

	frag = db[i];
	dev_block_put (db, bsize);
	return frag;
}

static int32_t
ufs1_inode_block_i2 (const struct ufs1_sb *sb, const struct ufs1_inode *o,
		     uint64_t i0, unsigned order, size_t mask)
{
	const uint64_t i1 = i0 >> order, i2 = i1 >> order, i3 = i2 >> order;
	int32_t l0, l1, l2 = o->i_ib[2];

	return	i3 != 0 ? 0 :
		(l1 = ufs1_block_map (sb, o, l2, order, i2       )) <= 0 ? l1 :
		(l0 = ufs1_block_map (sb, o, l1, order, i1 & mask)) <= 0 ? l0 :
		(     ufs1_block_map (sb, o, l0, order, i0 & mask));
}

static int32_t
ufs1_inode_block_i1 (const struct ufs1_sb *sb, const struct ufs1_inode *o,
		     uint64_t i0, unsigned order, uint64_t count)
{
	const size_t mask = ((size_t) 1 << order) - 1;
	const uint64_t i1 = i0 >> order;
	int32_t l0, l1 = o->i_ib[1];

	return	i0 >= count ?
		ufs1_inode_block_i2 (sb, o, i0 - count, order, mask) :
		(l0 = ufs1_block_map (sb, o, l1, order, i1       )) <= 0 ? l0 :
		(     ufs1_block_map (sb, o, l0, order, i0 & mask));
}

static int32_t
ufs1_inode_block_i0 (const struct ufs1_sb *sb, const struct ufs1_inode *o,
		     uint64_t i0, unsigned order)
{
	const uint64_t count = (uint64_t) 1 << order;
	int32_t l0 = o->i_ib[0];

	return	i0 >= count ?
		ufs1_inode_block_i1 (sb, o, i0 - count, order, count << order) :
		ufs1_block_map (sb, o, l0, order, i0);
}

int32_t ufs1_inode_block (const struct ufs1_sb *sb, const struct ufs1_inode *o,
			  uint64_t i)
{
	const size_t count = ARRAY_SIZE (o->i_db);

	return	o->i_size <= 0 ? 0 : i < count ? o->i_db[i] :
		ufs1_inode_block_i0 (sb, o, i - count, sb->bshift - 2);
}
