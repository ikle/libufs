/*
 * UNIX File System v1 On-Disk Super Block
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_FS_UFS1_SB_H
#define SYS_FS_UFS1_SB_H  1

#include <fs/ufs1-stat.h>

#define UFS1_SB_MAGIC	0x00011954

/*
 * On-disk SB format version 2
 *
 * 1. Introduced in BSD 4.1b.
 * 2. New rotations tables format introduced in 4.3BSD-Tahoe.
 * 3. Quad mask hints added in 4.3BSD-Tahoe.
 * 4. FreeBSD 5.0 deprecates rotation tables, s_csmask and s_csshift.
 * 5. Volume name and system-wide unique ID introduced in FreeBSD 6.0.
 *
 * Legend:
 *
 * 1. (-) -- deprecated, do not change initial values.
 * 2.  +  -- core paramemper,
 * 3. (h) -- hint, can be computed from core parameters.
 * 4. (c) -- config: user preferences, affect writes.
 * 5. (i) -- config: filesystem identification.
 * 6. (s) -- filesystem state: timestamp or statistics.
 */
struct ufs1_sb {
	int32_t		s_link;		/* (-) linked list of FSs	*/
	int32_t		s_rlink;	/* (-) linked list of SBs	*/
	int32_t		s_sblkno;	/* first SB fragment in CG	*/
	int32_t		s_cblkno;	/*  +  first CG fragment in CG	*/
	int32_t		s_iblkno;	/*  +  first inode frag in CG	*/
	int32_t		s_dblkno;	/*  +  first data frag in CG	*/
	int32_t		s_cgoffset;	/* (-) CG offset in cylinder	*/
	int32_t		s_cgmask;	/* (-) used to calc mod s_ntrak	*/
	uint32_t	s_time;		/* (s) last write timestamp	*/
	int32_t		s_size;		/*  +  number of blocks in FS	*/
	int32_t		s_dsize;	/* (h) num of data blocks in FS	*/
	uint32_t	s_ncg;		/*  +  number of CGs		*/
	int32_t		s_bsize;	/* (h) = (1 << s_bshift)	*/
	int32_t		s_fsize;	/* (h) = (1 << s_fshift)	*/
	int32_t		s_frag;		/* (h) = (s_bsize / s_fsize)	*/
	int32_t		s_minfree;	/* (c) minimum free space	*/
/* 0x40 */
	int32_t		s_rotdelay;	/* (-) ms for optimal next sect	*/
	int32_t		s_rps;		/* (-) disk rotation speed	*/
	int32_t		s_bmask;	/* (d) = (~0 << s_bshift)	*/
	int32_t		s_fmask;	/* (d) = (~0 << s_fshift)	*/
	int32_t		s_bshift;	/*  +  = ilog2 (size of block)	*/
	int32_t		s_fshift;	/*  +  = ilog2 (size of frag)	*/
	int32_t		s_maxcontig;	/* (c) max cluster length	*/
	int32_t		s_maxbpg;	/* (c) max indirect blks per CG	*/
	int32_t		s_fragshift;	/* (h) = (s_bshift - s_fshift)	*/
	int32_t		s_fsbtodb;	/* (h) = (s_fshift - 9)		*/
	int32_t		s_sbsize;	/* (h) actual size of SB	*/
	int32_t		s_csmask;	/* (-) = (~0 << s_csshift)	*/
	int32_t		s_csshift;	/* (-) = ilog2 (csum size) = 4	*/
	int32_t		s_nindir;	/* (c) count of indirect levels	*/
	uint32_t	s_inopb;	/* (h) = (s_bsize / 128)	*/
	int32_t		s_nspf;		/* (-) = (s_fsize / 512)	*/
/* 0x80 */
	int32_t		s_optim;	/* (c) optimization preference	*/
	int32_t		s_npsect;	/* (-) sectors per track	*/
	int32_t		s_interleave;	/* (-) sector interleave	*/
	int32_t		s_trackskew;	/* (-) sector 0 skew per track	*/
	int32_t		s_id[2];	/* (i) unique filesystem id	*/
	int32_t		s_csaddr;	/* (?) first frag of CG summary	*/
	int32_t		s_cssize;	/* (?) size of CG summary area	*/
	int32_t		s_cgsize;	/* (h) actual size of CG struct	*/
	int32_t		s_ntrak;	/* (-) tracks per cylinder	*/
	int32_t		s_nsect;	/* (-) sectors per track	*/
	int32_t		s_spc;		/* (-) sectors per cylinder	*/
	int32_t		s_ncyl;		/* (-) cylinder count		*/
	int32_t		s_cpg;		/* (-) cylinders per CG		*/
	uint32_t	s_ipg;		/* (h) = (dblkno-iblkno)/inopb	*/
	int32_t		s_fpg;		/*  +  fragments per CG		*/
/* 0xC0 */
	struct ufs1_cs	s_cstotal;	/* (h) FS statistics		*/
	int8_t		s_fmod;		/* (-) SB modified flag		*/
	int8_t		s_clean;	/* (-) FS is clean flag		*/
	int8_t		s_ronly;	/* (-) mounted read-only flag	*/
	int8_t		s_flags;	/* (-) FS state flags		*/
	uint8_t		s_root[468];	/* (-) current mount point	*/
	uint8_t		s_volname[32];	/* (i) volume name		*/
	uint64_t	s_swuid;	/* (i) system-wide unique id	*/
	int32_t		s_pad;
	int32_t		s_cgrotor;	/* (h) last CG searched		*/
	int32_t		s_csp[32];	/* (-) in-core cs info buffers	*/
	int32_t		s_cpc;		/* (-) cyl per cycle in postbl	*/
	int16_t		s_opostbl[128];	/* (-) rot. block list head	*/
	int32_t		s_sparecon[50];
	int32_t		s_contigsumlen;	/* (c) len of cluster sum array	*/
	int32_t		s_maxembedded;	/*  +  = 60, symlinks only	*/
	int32_t		s_inodefmt;	/*  +  = 2 for 4.4BSD UFS1	*/
	uint64_t	s_maxfilesize;	/* (c) maximum file size	*/
	int64_t		s_qbmask;	/* (h) = (s_bsize - 1)		*/
	int64_t		s_qfmask;	/* (h) = (s_fsize - 1)		*/
	int32_t		s_state;	/* (-) FS state time stamp	*/
	int32_t		s_postblformat;	/* (-) = 1 aka 4.4BSD format	*/
	int32_t		s_nrpos;	/* (-) = 1, # of rot. positions	*/
	int32_t		s_postbloff;	/* (-) rot. block list head	*/
	int32_t		s_rotbloff;	/* (-) blocks for each rotation	*/
	int32_t		s_magic;
/* 0x560 */
};

/*
 * Next set of functions returns fragment number
 */
static inline int32_t ufs1_cg_base (const struct ufs1_sb *s, uint32_t cgx)
{
	return s->s_fpg * cgx;
}

static inline int32_t ufs1_cg_start (const struct ufs1_sb *s, uint32_t cgx)
{
	return ufs1_cg_base (s, cgx) + s->s_cgoffset * (cgx & ~s->s_cgmask);
}

static inline int32_t ufs1_cg_sblkno (const struct ufs1_sb *s, uint32_t cgx)
{
	return ufs1_cg_start (s, cgx) + s->s_sblkno;
}

static inline int32_t ufs1_cg_cblkno (const struct ufs1_sb *s, uint32_t cgx)
{
	return ufs1_cg_start (s, cgx) + s->s_cblkno;
}

static inline int32_t ufs1_cg_iblkno (const struct ufs1_sb *s, uint32_t cgx)
{
	return ufs1_cg_start (s, cgx) + s->s_iblkno;
}

static inline int32_t ufs1_cg_dblkno (const struct ufs1_sb *s, uint32_t cgx)
{
	return ufs1_cg_start (s, cgx) + s->s_dblkno;
}

#endif  /* SYS_FS_UFS1_SB_H */
