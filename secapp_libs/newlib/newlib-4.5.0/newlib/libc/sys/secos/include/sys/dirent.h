// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* FIXME: From sys/tsvc/sys */
#ifndef _SYS_DIRENT_H
# define _SYS_DIRENT_H

/* linux-compatible place-holder */

#include <sys/types.h>

typedef struct {
} DIR;

/* copied from 'man readdir' */
struct dirent {
  ino_t d_ino;
  off_t d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[256];
};

/* File types for `d_type'.  */
enum
{
  DT_UNKNOWN = 0,
# define DT_UNKNOWN     DT_UNKNOWN
  DT_FIFO = 1,
# define DT_FIFO        DT_FIFO
  DT_CHR = 2,
# define DT_CHR         DT_CHR
  DT_DIR = 4,
# define DT_DIR         DT_DIR
  DT_BLK = 6,
# define DT_BLK         DT_BLK
  DT_REG = 8,
# define DT_REG         DT_REG
  DT_LNK = 10,
# define DT_LNK         DT_LNK
  DT_SOCK = 12,
# define DT_SOCK        DT_SOCK
  DT_WHT = 14
# define DT_WHT         DT_WHT
};

#endif