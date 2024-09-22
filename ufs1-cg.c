/*
 * UFS1 Cylinder Group
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sys/param.h>

#include <marten/device/block.h>
#include <fs/ufs1-cg-v2.h>

#include "ufs1-cg.h"

void ufs1_cg_fini (struct ufs1_cg *o)
{
	dev_block_put (o->data, o->sb->cgsize);
}

static inline int ufs1_cg_error (struct ufs1_cg *o, const char *reason)
{
	ufs1_cg_fini (o);
	return 0;
}

int ufs1_cg_init (struct ufs1_cg *o, struct ufs1_sb *s, uint32_t cgx)
{
	const off_t pos = (off_t) ufs1_cg_cblkno (o->sb = s, cgx) << s->fshift;
	struct ufs1_cg_v2 *c;

	if ((c = o->data = dev_block_get (s->dev, pos, s->cgsize, 1)) == NULL)
		return 0;

	if (c->cg_magic != UFS1_CG_MAGIC)
		return ufs1_cg_error (o, "Cannot find valid cylinder group magic");

	o->start = ufs1_cg_start (s, cgx);
	o->cgx   = c->cg_cgx;
	o->ipg   = c->cg_ipg;
	o->fpg   = c->cg_fpg;

	if (o->cgx != cgx || o->ipg != s->ipg || o->fpg > s->fpg)
		return ufs1_cg_error (o, "Invalid cylinder group configuration");

	o->imap_pos = c->cg_iusedoff;
	o->fmap_pos = c->cg_freeoff;
	o->emap_pos = c->cg_nextfreeoff;

	if (o->emap_pos > s->cgsize || o->fmap_pos >= o->emap_pos ||
	    o->imap_pos >= o->fmap_pos ||
	    (o->fmap_pos - o->imap_pos) < howmany (o->ipg, 8) ||
	    (o->emap_pos - o->fmap_pos) < howmany (o->fpg, 8))
		return ufs1_cg_error (o, "Invalid cylinder group layout");

	o->stat = c->cg_cs;
	return 1;
}
