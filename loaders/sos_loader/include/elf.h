// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// copy from xv6

// Format of an ELF executable file
#ifndef SOSLD_ELF_H
#define SOSLD_ELF_H

#include <app_types.h>
#include <vk_mm_defs.h>

#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian




#ifdef __i386__

// File header
struct elfhdr
{
	uint32_t magic; // must equal ELF_MAGIC
	uint8_t elf[12];
	uint16_t type; // 0x10
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
	uint8_t __reserved[PAGE_SIZE_4K - 52];
};

// Program section header
struct proghdr
{
	uint32_t type;
	uint32_t off;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
};

#elif defined(__amd64__)

// File header
struct elfhdr
{
	uint32_t magic; // must equal ELF_MAGIC
	uint8_t elf[12];
	uint16_t type; // 0x10
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
	uint8_t __reserved[PAGE_SIZE_4K - 52];
};

// Program section header
struct proghdr
{
	uint32_t type;
	uint32_t flags;
	uint64_t off;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t filesz;
	uint64_t memsz;
	uint64_t align;
};

#else /* !defined(__i386__) && !defined(__amd64__) */
#error "Unsupported Arch"
#endif /* !defined(__i386__) && !defined(__amd64__) */

// Values for Proghdr type
#define ELF_PROG_LOAD 1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC 1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ 4

#endif // SOSLD_ELF_H