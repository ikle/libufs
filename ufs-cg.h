/*
 * UFS Cylinder Group
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef UFS_CG_H
#define UFS_CG_H  1

#include <stdlib.h>

#include <sys/param.h>
#include <unistd.h>

#include "ufs-sb.h"

struct ufs_cg {
	const struct ufs_sb *sb;

	void		*cg_data;
	int32_t		cg_start;
	uint32_t	cg_cgx, cg_ipg, cg_fpg;
	uint32_t	cg_imap_pos, cg_fmap_pos, cg_emap_pos;
	struct ufs1_cs	cg_stat;
};

static inline uint8_t *ufs_cg_imap (const struct ufs_cg *o)
{
	return o->cg_data + o->cg_imap_pos;
}

static inline uint8_t *ufs_cg_fmap (const struct ufs_cg *o)
{
	return o->cg_data + o->cg_fmap_pos;
}

static inline void ufs_cg_fini (struct ufs_cg *o)
{
	free (o->cg_data);
}

static inline int ufs_cg_error (struct ufs_cg *o, const char *reason)
{
	free (o->cg_data);
	return 0;
}

static inline
int ufs_cg_init (struct ufs_cg *o, const struct ufs_sb *s, uint32_t cgx)
{
	struct ufs1_cg *c;
	off_t pos = (off_t) ufs_cg_cblkno (o->sb = s, cgx) << s->s_fshift;

	if ((c = o->cg_data = malloc (s->s_cgsize)) == NULL)
		return 0;

	if (pread (s->fd, c, s->s_cgsize, pos) != s->s_cgsize)
		return ufs_cg_error (o, "Cannot read cylinder group");

	if (c->cg_magic != UFS1_CG_MAGIC)
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

static inline uint32_t ufs_cg_ino (const struct ufs_cg *o, uint32_t i)
{
	return o->sb->s_ipg * o->cg_cgx + i;
}

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
