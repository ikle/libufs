/*
 * UNIX File System v1 On-Disk Cylinder Group
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_FS_UFS1_CG_H
#define SYS_FS_UFS1_CG_H  1

#include <fs/ufs1-sb.h>

#define UFS1_CG_MAGIC	0x00090255

/*
 * On-disk CG format version 2
 *
 * 1. Introduced in BSD 4.3-Tahoe.
 * 2. Magic field position moved.
 * 3. Added support for more than 32 cylinders per CG.
 * 4. FreeBSD 2.0.5 adds cluster support.
 * 5. FreeBSD 5.0 removes support for cg_ncyl, cg_btot and cg_b.
 */
struct ufs1_cg_v2 {
	int32_t		cg_link;	/* (-) linked list of CGs	*/
	int32_t		cg_magic;
	uint32_t	cg_time;	/* (s) time last written	*/
	uint32_t	cg_cgx;		/* this cylinder group index	*/
	int16_t		cg_ncyl;	/* (-) number of cylinders	*/
	int16_t		cg_ipg;		/* (h) number of inodes (s_ipg)	*/
	int32_t		cg_fpg;		/*  +  number of data fragments	*/
	struct ufs1_cs	cg_cs;		/* (s) CG summary info		*/
	int32_t		cg_rotor;	/* (h) pos. of last used block	*/
	int32_t		cg_frotor;	/* (h) pos. of last used frag	*/
	int32_t		cg_irotor;	/* (h) pos. of last used inode	*/
	int32_t		cg_frsum[8];	/* (h) count of available frags	*/
	uint32_t	cg_btotoff;	/* (-) block totals per cyl.	*/
	uint32_t	cg_boff;	/* (-) free blocks positions	*/
	uint32_t	cg_iusedoff;	/*  +  used inode map		*/
	uint32_t	cg_freeoff;	/*  +  free block map		*/
	uint32_t	cg_nextfreeoff;	/* (h) next available space	*/
	uint32_t	cg_clustersumoff; /* counts of avail clusters	*/
	uint32_t	cg_clusteroff;	/* free cluster map		*/
	uint32_t	cg_nclusterblks;/* number of clusters		*/
	int32_t		cg_sparecon[13];
	uint8_t		cg_space[];	/* space for CG maps		*/
};

static inline int32_t *ufs1_cg_v2_btot (struct ufs1_cg_v2 *o)
{
	return (void *) o + o->cg_btotoff;	/* [s_cpg] */
}

static inline
int16_t *ufs1_cg_b (const struct ufs1_sb_v2 *s, struct ufs1_cg_v2 *o, int cylno)
{
	return (int16_t *) ((void *) o + o->cg_boff) + s->s_nrpos * cylno;  /* [s_cpg * s_nrpos] */
}

static inline uint8_t *ufs1_cg_v2_imap (struct ufs1_cg_v2 *o)
{
	return (void *) o + o->cg_iusedoff;	/* [(s_ipg + 7) / 8] */
}

static inline uint8_t *ufs1_cg_v2_dmap (struct ufs1_cg_v2 *o)
{
	return (void *) o + o->cg_freeoff;	/* [(s_fpg + 7) / 8] */
}

static inline int32_t *ufs1_cg_v2_cstat (struct ufs1_cg_v2 *o)
{
	return (void *) o + o->cg_clustersumoff;  /* [s_contigsumsize] */
}

static inline uint8_t *ufs1_cg_v2_cmap (struct ufs1_cg_v2 *o)
{
	return (void *) o + o->cg_clusteroff;	/* [cg_nclusterblks] */
}

#endif  /* SYS_FS_UFS1_CG_H */
