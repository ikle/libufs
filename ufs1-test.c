/*
 * UNIX File System v1 Test
 *
 * Copyright (c) 2023 Alexei A. Smekalkine <ikle@ikle.ru>
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

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof (a) / sizeof ((a)[0]))
#endif

struct ufs_sb {
	int		fd;
	uint32_t	s_sblkno, s_cblkno, s_iblkno, s_dblkno;
	int32_t		s_cgoffset, s_cgmask;
	uint32_t	s_ncg, s_bshift, s_fshift, s_inopb;
	uint32_t	s_cgsize, s_ipg, s_fpg;
	struct ufs1_cs	s_stat;
};

static int ufs_sb_init (struct ufs_sb *o, int fd)
{
	struct ufs1_sb buf, *s = &buf;

	if (pread (fd, &buf, sizeof (buf), 8192) != sizeof (buf))
		return 0;

	o->fd = fd;

	if (s->s_magic != UFS1_SB_MAGIC)
		return 0;

	o->s_sblkno   = s->s_sblkno;
	o->s_cblkno   = s->s_cblkno;
	o->s_iblkno   = s->s_iblkno;
	o->s_dblkno   = s->s_dblkno;
	o->s_cgsize   = s->s_cgsize;
	o->s_cgoffset = s->s_cgoffset;
	o->s_cgmask   = s->s_cgmask;
	o->s_ncg      = s->s_ncg;
	o->s_ipg      = s->s_ipg;
	o->s_fpg      = s->s_fpg;

	if (o->s_sblkno >= o->s_cblkno || o->s_cblkno >= o->s_iblkno ||
	    o->s_iblkno >= o->s_dblkno || o->s_dblkno >= o->s_fpg ||
	    o->s_cgsize < sizeof (struct ufs1_cg) ||
	    o->s_cgsize > (o->s_iblkno - o->s_cblkno) << s->s_fshift)
		return 0;

	o->s_bshift = s->s_bshift;
	o->s_fshift = s->s_fshift;
	o->s_inopb  = s->s_inopb;

	if (s->s_bshift < 12 || s->s_bsize != (1L << o->s_bshift) ||
	    s->s_fshift < 9  || s->s_fsize != (1L << o->s_fshift) ||
	    s->s_fragshift != (o->s_bshift - o->s_fshift) ||
	    s->s_fragshift < 0 || s->s_fragshift > 3 ||
	    s->s_fsbtodb != (o->s_fshift - 9) ||
	    s->s_frag != (1L << (o->s_bshift - o->s_fshift)) ||
	    s->s_bmask != (~0L << o->s_bshift) ||
	    s->s_fmask != (~0L << o->s_fshift) ||
	    o->s_inopb != (s->s_bsize / 128))
		return 0;

	if (s->s_maxembedded != 60 || s->s_inodefmt != 2)
		return 0;

	o->s_stat = s->s_cstotal;
	return 1;
}

static void ufs_sb_fini (struct ufs_sb *o)
{
	close (o->fd);
}

struct ufs_cg {
	void		*cg_data;
	int32_t		cg_start;
	uint32_t	cg_cgx, cg_ipg, cg_fpg;
	uint32_t	cg_imap_pos, cg_fmap_pos, cg_emap_pos;
	struct ufs1_cs	cg_stat;
};

static inline int32_t ufs_cg_start (const struct ufs_sb *o, uint32_t cgx)
{
	return o->s_fpg * cgx + o->s_cgoffset * (cgx & ~o->s_cgmask);
}

static inline int32_t ufs_cg_cblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->s_cblkno;
}

static inline int32_t ufs_cg_iblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->s_iblkno;
}

static inline int32_t ufs_cg_dblkno (const struct ufs_sb *o, uint32_t cgx)
{
	return ufs_cg_start (o, cgx) + o->s_dblkno;
}

static inline uint8_t *ufs_cg_imap (const struct ufs_cg *o)
{
	return o->cg_data + o->cg_imap_pos;
}

static inline uint8_t *ufs_cg_fmap (const struct ufs_cg *o)
{
	return o->cg_data + o->cg_fmap_pos;
}

static int ufs_cg_init (struct ufs_cg *o, const struct ufs_sb *s, uint32_t cgx)
{
	struct ufs1_cg *c;
	off_t pos = (off_t) ufs_cg_cblkno (s, cgx) << s->s_fshift;

	if ((c = o->cg_data = malloc (s->s_cgsize)) == NULL)
		return 0;

	if (pread (s->fd, c, s->s_cgsize, pos) != s->s_cgsize)
		goto error;

	if (c->cg_magic != UFS1_CG_MAGIC)
		goto error;

	o->cg_start = ufs_cg_start (s, cgx);
	o->cg_cgx   = c->cg_cgx;
	o->cg_ipg   = c->cg_ipg;
	o->cg_fpg   = c->cg_fpg;

	if (o->cg_ipg != s->s_ipg || o->cg_fpg > s->s_fpg)
		goto error;

	o->cg_imap_pos = c->cg_iusedoff;
	o->cg_fmap_pos = c->cg_freeoff;
	o->cg_emap_pos = c->cg_nextfreeoff;

	if (o->cg_emap_pos > s->s_cgsize || o->cg_fmap_pos >= o->cg_emap_pos ||
	    o->cg_imap_pos >= o->cg_fmap_pos ||
	    (o->cg_fmap_pos - o->cg_imap_pos) < howmany (o->cg_ipg, 8) ||
	    (o->cg_emap_pos - o->cg_fmap_pos) < howmany (o->cg_fpg, 8))
		goto error;

	o->cg_stat = c->cg_cs;
	return 1;
error:
	free (o->cg_data);
	return 0;
}

static void ufs_cg_fini (struct ufs_cg *o)
{
	free (o->cg_data);
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

static
int ufs1_inode_show (const struct ufs_sb *s, const struct ufs_cg *c, int n)
{
	struct ufs1_inode buf, *o = &buf;
	off_t pos = ((off_t) ufs_cg_iblkno (s, c->cg_cgx) << s->s_fshift) +
		    n * sizeof (*o);
	uint32_t ino = s->s_ipg * c->cg_cgx + n;

	if (!isset (ufs_cg_imap (c), n))
		return 1;

	if (pread (s->fd, &buf, sizeof (buf), pos) != sizeof (buf))
		return 0;

	fprintf (stderr, "N:     %2d: %06o %3d %4u %4u %8llu, %3u sectors",
		 ino, o->i_mode, o->i_nlink, o->i_uid, o->i_gid,
		 (unsigned long long) o->i_size, o->i_blocks);
	ufs1_inode_show_db (s, o);
	fputc ('\n', stderr);
	return 1;
}

static int ufs_cg_show_inodes (const struct ufs_sb *s, const struct ufs_cg *c)
{
	uint32_t i;

	fprintf (stderr, "I: List of i-nodes:\n");

	for (i = 0; i < c->cg_ipg; ++i)
		if (!ufs1_inode_show (s, c, i))
			return 0;

	return 1;
}

static void show_stat (const struct ufs1_cs *o)
{
	fprintf (stderr, "N:     directories = %d\n", o->cs_ndir);
	fprintf (stderr, "N:     free blocks = %d\n", o->cs_nbfree);
	fprintf (stderr, "N:     free inodes = %d\n", o->cs_nifree);
	fprintf (stderr, "N:     free frags  = %d\n", o->cs_nffree);
}

int main (int argc, char *argv[])
{
	int fd;
	struct ufs_sb s;
	struct ufs_cg c;
	uint32_t i;

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
		goto no_sb;
	}

	fprintf (stderr, "I: Valid UFS1 super block found\n");
	fprintf (stderr, "N:     block size  = %d\n", 1 << s.s_bshift);
	fprintf (stderr, "N:     frag size   = %d\n", 1 << s.s_fshift);
	show_stat (&s.s_stat);

	for (i = 0; i < s.s_ncg; ++i) {
		if (!ufs_cg_init (&c, &s, i)) {
			fprintf (stderr, "E: Cannot find valid UFS1 "
					 "cylinder group %u\n", i);
			goto no_cg;
		}

		fprintf (stderr, "I: Valid UFS1 cylinder group %u found\n", i);
		show_stat (&c.cg_stat);
		ufs_cg_show_inodes (&s, &c);
		ufs_cg_fini (&c);
	}

	ufs_sb_fini (&s);
	return 0;
no_cg:
	ufs_cg_fini (&c);
no_sb:
	ufs_sb_fini (&s);
	return 1;
}
