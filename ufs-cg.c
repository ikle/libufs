/*
 * UFS Cylinder Group
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sys/param.h>

#include <marten/device/block.h>

#include "ufs-cg.h"

void ufs_cg_fini (struct ufs_cg *o)
{
	dev_block_put (o->cg_data, o->sb->s_cgsize);
}

static inline int ufs_cg_error (struct ufs_cg *o, const char *reason)
{
	ufs_cg_fini (o);
	return 0;
}

int ufs_cg_init (struct ufs_cg *o, const struct ufs_sb *s, uint32_t cgx)
{
	const off_t pos = (off_t) ufs_cg_cblkno (o->sb = s, cgx) << s->s_fshift;
	struct ufs1_cg_v2 *c;

	if ((o->cg_data = dev_block_get (s->dev, pos, s->s_cgsize, 1)) == NULL)
		return 0;

	if ((c = o->cg_data)->cg_magic != UFS1_CG_MAGIC)
		return ufs_cg_error (o, "Cannot find valid cylinder group magic");

	o->cg_start = ufs_cg_start (s, cgx);
	o->cg_cgx   = c->cg_cgx;
	o->cg_ipg   = c->cg_ipg;
	o->cg_fpg   = c->cg_fpg;

	if (o->cg_cgx != cgx || o->cg_ipg != s->s_ipg || o->cg_fpg > s->s_fpg)
		return ufs_cg_error (o, "Invalid cylinder group configuration");

	o->cg_imap_pos = c->cg_iusedoff;
	o->cg_fmap_pos = c->cg_freeoff;
	o->cg_emap_pos = c->cg_nextfreeoff;

	if (o->cg_emap_pos > s->s_cgsize || o->cg_fmap_pos >= o->cg_emap_pos ||
	    o->cg_imap_pos >= o->cg_fmap_pos ||
	    (o->cg_fmap_pos - o->cg_imap_pos) < howmany (o->cg_ipg, 8) ||
	    (o->cg_emap_pos - o->cg_fmap_pos) < howmany (o->cg_fpg, 8))
		return ufs_cg_error (o, "Invalid cylinder group layout");

	o->cg_stat = c->cg_cs;
	return 1;
}
