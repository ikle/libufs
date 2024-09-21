/*
 * UNIX File System v1 On-Disk I-Node, 128 bytes
 *
 * Copyright (c) 2023-2024 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_FS_UFS1_INODE_H
#define SYS_FS_UFS1_INODE_H  1

#include <stdint.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof (a) / sizeof ((a)[0]))
#endif

union ufs1_data {
	struct {
		int32_t db[12];		/* direct disk blocks		*/
		int32_t ib[3];		/* indirect disk blocks		*/
	};
	uint32_t rdev;			/* device number (special file)	*/
	uint8_t  content[60];		/* embedded data		*/
};

/*
 * On-disk i-node format version 2
 *
 * 1. Introduced in 4.4BSD, used if fs_inodefmt = 2 (FS_44INODEFMT) and
 *    fs_maxsymlinklen = 60 (sizeof (i_content)).
 * 2. Rationale for the fs_maxsymlinklen check: it shares position with
 *    Solaris fs_version which is in range [0, 2], and fs_inodefmt shares
 *    position with Solaris fs_logbno (this probably should never be equal
 *    to 2 but who knows).
 * 3. User identifiers moved to new position to be 32-bit wide.
 * 4. The index fields i_db and i_ib reused for short symlinks.
 * 5. FreeBSD 3.3 adds support for Soft Update Journal (SUJ), FreeBSD 9.0
 *    reuses old ids place as i_freelink.
 * 6. FreeBSD 8.0 adds i_modrev to support NFSv4.
 */
struct ufs1_inode {
	uint16_t	i_mode;		/* file type and permissions	*/
	uint16_t	i_nlink;	/* file name count		*/
	uint32_t	i_freelink;	/* SUJ: next unlinked inode	*/
	uint64_t	i_size;		/* file size in bytes		*/
	uint32_t	i_atime;	/* last access time		*/
	uint32_t	i_atime_ns;
	uint32_t	i_mtime;	/* last modified time		*/
	uint32_t	i_mtime_ns;
	uint32_t	i_ctime;	/* last inode change time	*/
	uint32_t	i_ctime_ns;
	union ufs1_data	i_data;
	uint32_t	i_flags;	/* status flags			*/
	uint32_t	i_blocks;	/* (h) allocated sectors	*/
	uint32_t	i_gen;		/* NFS: generation number	*/
	uint32_t	i_uid;
	uint32_t	i_gid;
	uint64_t	i_modrev;	/* NFSv4: mode revision?	*/
};

#ifndef i_db
#define i_db		i_data.db
#define i_ib		i_data.ib
#define i_rdev		i_data.rdev
#define i_content	i_data.content
#endif

static inline uint32_t ufs1_major (uint32_t rdev)
{
	return (rdev >> 8) & 0xff;
}

static inline uint32_t ufs1_minor (uint32_t rdev)
{
	return (rdev & 0xff) | ((rdev >> 8) & 0xffff00);
}

static inline uint32_t ufs1_makedev (uint32_t major, uint32_t minor)
{
	return (major << 8) | (minor & 0xff) | (minor & 0xffff00) << 8;
}

#endif  /* SYS_FS_UFS1_INODE_H */
