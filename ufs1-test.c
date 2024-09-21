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

#include <fs/ufs1-dirent.h>
#include <fs/ufs1-inode.h>

#include "dev-block.h"
#include "ufs-cg.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof (a) / sizeof ((a)[0]))
#endif

static void *
ufs1_inode_dir_pull (const struct ufs_cg *c, const struct ufs1_inode *o,
		     uint64_t frag)
{
	const uint64_t head  = (frag + 0) * UFS1_DFSIZE;
	const uint64_t next  = (frag + 1) * UFS1_DFSIZE;
	const uint32_t block = head >> c->sb->s_bshift;
	const uint32_t offs  = head & ~(~0 << c->sb->s_bshift);
	off_t pos;

	if (next > o->i_size)
		return NULL;

	if (block >= ARRAY_SIZE (o->i_db))
		return NULL;  /* indirect blocks not supported yet */

	pos = ((off_t) o->i_db[block] << c->sb->s_fshift) + offs;

	return dev_block_get (c->sb->fd, pos, UFS1_DFSIZE, 1);
}

static void ufs1_dirent_show (const struct ufs1_dirent *o)
{
	if (o->d_ino != 0 && o->d_namlen > 0)
		fprintf (stderr, "I:          %2d: %.*s\n",
			 o->d_ino, o->d_namlen, o->d_name);
}

static void ufs1_dirent_show_frag (const void *frag)
{
	const void *p, *end;

	for (
		p = frag, end = p + UFS1_DFSIZE;
		ufs1_dirent_valid (p, end - p);
		p = ufs1_dirent_next (p)
	)
		ufs1_dirent_show (p);
}

static void ufs1_dir_show (const struct ufs_cg *c, const struct ufs1_inode *o)
{
	uint64_t i;
	void *p;

	for (i = 0; (p = ufs1_inode_dir_pull (c, o, i)) != NULL; ++i) {
		ufs1_dirent_show_frag (p);
		dev_block_put (p, UFS1_DFSIZE);
	}
}

static void ufs1_show_mode (unsigned mode, FILE *to)
{
	const char *map = "0fc3d5b7-9lBsDwF";
	const int type = IFTODT (mode);
	const int suid = (mode & 04000) != 0;
	const int sgid = (mode & 02000) != 0;
	const int svtx = (mode & 01000) != 0;

	fputc (map[type], to);

	fputc (mode & 0400 ? 'r' : '-', to);
	fputc (mode & 0200 ? 'w' : '-', to);
	fputc (mode & 0100 ? suid ? 's' : 'x' : suid ? 'S' : '-', to);

	fputc (mode & 0040 ? 'r' : '-', to);
	fputc (mode & 0020 ? 'w' : '-', to);
	fputc (mode & 0010 ? sgid ? 's' : 'x' : sgid ? 'S' : '-', to);

	fputc (mode & 0004 ? 'r' : '-', to);
	fputc (mode & 0002 ? 'w' : '-', to);
	fputc (mode & 0001 ? svtx ? 't' : 'x' : svtx ? 'T' : '-', to);
}

static
void ufs1_inode_show_blocks (const struct ufs_cg *c, const struct ufs1_inode *o)
{
	const uint64_t count  = howmany (o->i_size, 1u << c->sb->s_bshift);
	const size_t count_l1 = MIN (ARRAY_SIZE (o->i_db), count);
	size_t i;

	if (o->i_size == 0)
		return;

	fprintf (stderr, " at %d", o->i_db[0]);

	for (i = 1; i < count_l1; ++i)
		fprintf (stderr, ", %d", o->i_db[i]);

	if (count > count_l1)
		fprintf (stderr, ", ...");
}

static
void ufs1_inode_show_link (const struct ufs_cg *c, const struct ufs1_inode *o)
{
	if (o->i_size < 60 && o->i_content[o->i_size] == '\0')
		fprintf (stderr, " -> %s\n", o->i_content);
	else
		ufs1_inode_show_blocks (c, o);
}

static
void ufs1_inode_show_rdev (const struct ufs_cg *c, const struct ufs1_inode *o)
{
	fprintf (stderr, " -> %u, %u\n",
		 ufs1_major (o->i_rdev), ufs1_minor (o->i_rdev));
}

static
void ufs1_inode_show_data (const struct ufs_cg *c, const struct ufs1_inode *o)
{
	switch (IFTODT (o->i_mode)) {
	case DT_LNK:	ufs1_inode_show_link   (c, o); break;
	case DT_CHR:
	case DT_BLK:	ufs1_inode_show_rdev   (c, o); break;
	default:	ufs1_inode_show_blocks (c, o); break;
	}

	fputc ('\n', stderr);

	if (IFTODT (o->i_mode) == DT_DIR)
		ufs1_dir_show (c, o);
}

static int ufs1_cg_inode_show (const struct ufs_cg *c, int n)
{
	struct ufs1_inode buf, *o;

	if (!isset (ufs_cg_imap (c), n))
		return 1;

	if ((o = ufs1_cg_inode_pull (c, n, &buf)) == NULL)
		return 0;

	fprintf (stderr, "I:     %2d: ", ufs_cg_ino (c, n));
	ufs1_show_mode (o->i_mode, stderr);

	fprintf (stderr, " %3d %4u %4u %8llu, %3u sectors",
		 o->i_nlink, o->i_uid, o->i_gid,
		 (unsigned long long) o->i_size, o->i_blocks);
	ufs1_inode_show_data (c, o);
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
	int ok = 1, i;

	fprintf (stderr, "N: Valid UFS1 cylinder group %u found\n", o->cg_cgx);
	ufs1_show_stat (&o->cg_stat);

	fprintf (stderr, "I: List of i-nodes:\n");

	for (i = 0; i < o->cg_ipg; ++i)
		if (!ufs1_cg_inode_show (o, i)) {
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
