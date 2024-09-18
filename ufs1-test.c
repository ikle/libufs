/*
 * UNIX File System v1 Test
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>

#include <fs/ufs1-cg.h>
#include <fs/ufs1-dirent.h>
#include <fs/ufs1-inode.h>

#include "ufs-sb.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof (a) / sizeof ((a)[0]))
#endif

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

static void ufs_cg_fini (struct ufs_cg *o)
{
	free (o->cg_data);
}

static int ufs_cg_error (struct ufs_cg *o, const char *reason)
{
	free (o->cg_data);
	return 0;
}

static int ufs_cg_init (struct ufs_cg *o, const struct ufs_sb *s, uint32_t cgx)
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

static
void ufs1_inode_show_db (const struct ufs_sb *s, const struct ufs1_inode *o)
{
	const int count = MIN (ARRAY_SIZE (o->i_db),
			       howmany (o->i_size, 1 << s->s_bshift));
	int i;

	if (o->i_size == 0)
		return;

	if (IFTODT (o->i_mode) == DT_LNK && o->i_size < 60 &&
	    o->i_content[o->i_size] == '\0') {
		fprintf (stderr, " -> %s", o->i_content);
		return;
	}

	fprintf (stderr, " at %d", o->i_db[0]);

	for (i = 1; i < count; ++i)
		fprintf (stderr, ", %d", o->i_db[i]);
}

static int ufs1_inode_show (const struct ufs_cg *c, int n)
{
	const struct ufs_sb *s = c->sb;
	struct ufs1_inode buf, *o = &buf;
	off_t pos = ((off_t) ufs_cg_iblkno (s, c->cg_cgx) << s->s_fshift) +
		    n * sizeof (*o);

	if (!isset (ufs_cg_imap (c), n))
		return 1;

	if (pread (s->fd, &buf, sizeof (buf), pos) != sizeof (buf))
		return 0;

	fprintf (stderr, "I:     %2d: %06o %3d %4u %4u %8llu, %3u sectors",
		 ufs_cg_ino (c, n), o->i_mode, o->i_nlink, o->i_uid, o->i_gid,
		 (unsigned long long) o->i_size, o->i_blocks);
	ufs1_inode_show_db (s, o);
	fputc ('\n', stderr);
	return 1;
}

static void ufs1_show_stat (const struct ufs1_cs *o)
{
	fprintf (stderr, "I:     directories = %d\n", o->cs_ndir);
	fprintf (stderr, "I:     free blocks = %d\n", o->cs_nbfree);
	fprintf (stderr, "I:     free inodes = %d\n", o->cs_nifree);
	fprintf (stderr, "I:     free frags  = %d\n", o->cs_nffree);
}

static void ufs_sb_show (const struct ufs_sb *o)
{
	fprintf (stderr, "N: Valid UFS1 super block found\n");
	fprintf (stderr, "I:     block size  = %d\n", 1 << o->s_bshift);
	fprintf (stderr, "I:     frag size   = %d\n", 1 << o->s_fshift);
	ufs1_show_stat (&o->s_stat);
}

static int ufs_cg_show (const struct ufs_cg *o)
{
	int ok = 1;
	uint32_t i;

	fprintf (stderr, "N: Valid UFS1 cylinder group %u found\n", o->cg_cgx);
	ufs1_show_stat (&o->cg_stat);

	fprintf (stderr, "I: List of i-nodes:\n");

	for (i = 0; i < o->cg_ipg; ++i)
		if (!ufs1_inode_show (o, i)) {
			fprintf (stderr, "E: Cannot read inode %u\n",
				 ufs_cg_ino (o, i));
			ok = 0;
			continue;
		}

	return ok;
}

static int ufs_fs_show (const struct ufs_sb *sb)
{
	int ok = 1;
	uint32_t i;
	struct ufs_cg c;

	ufs_sb_show (sb);

	for (i = 0; i < sb->s_ncg; ++i) {
		if (!ufs_cg_init (&c, sb, i)) {
			fprintf (stderr, "E: Cannot find valid UFS1 "
					 "cylinder group %u\n", i);
			ok = 0;
			continue;
		}

		ok &= ufs_cg_show (&c);
		ufs_cg_fini (&c);
	}

	return ok;
}

int main (int argc, char *argv[])
{
	int fd, ok;
	struct ufs_sb s;

	if (argc != 2) {
		fprintf (stderr, "usage:\n\tufs1-test <ufs1-image>\n");
		return 1;
	}

	if ((fd = open (argv[1], O_RDONLY)) == -1) {
		perror (argv[1]);
		return 1;
	}

	if (!ufs_sb_init (&s, fd)) {
		fprintf (stderr, "E: Cannot find valid UFS1 super block\n");
		return 1;
	}

	ok = ufs_fs_show (&s);

	ufs_sb_fini (&s);
	return ok ? 0 : 1;
}
