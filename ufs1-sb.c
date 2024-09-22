/*
 * UFS1 Super Block
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <unistd.h>

#include <fs/ufs1-cg.h>

#include "ufs1-sb.h"

void ufs1_sb_fini (struct ufs1_sb *o)
{
	close (o->dev);
}

static inline int ufs1_sb_error (struct ufs1_sb *o, const char *reason)
{
	ufs1_sb_fini (o);
	return 0;
}

int ufs1_sb_init (struct ufs1_sb *o, int dev)
{
	struct ufs1_sb_v2 buf, *s = &buf;

	if (pread (o->dev = dev, &buf, sizeof (buf), 8192) != sizeof (buf))
		return ufs1_sb_error (o, "Cannot read super block");

	if (s->s_magic != UFS1_SB_MAGIC)
		return ufs1_sb_error (o, "Cannot find valid super block magic");

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
		return ufs1_sb_error (o, "Invalid file system layout");

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
		return ufs1_sb_error (o, "Invalid file system configuration");

	if (s->s_maxembedded != 60 || s->s_inodefmt != 2)
		return ufs1_sb_error (o, "Unknown i-node format");

	o->stat = s->s_cstotal;
	return 1;
}
