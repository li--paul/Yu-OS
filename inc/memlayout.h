#ifndef YUOS_INC_MEMLAYOUT_H
#define YUOS_INC_MEMLAYOUT_H

#ifndef __ASSEMBLER__
#include <inc/types.h>
#include <inc/mmu.h>
#endif 	/* not __ASSEMBLER__ */

/*
 * This file contains definitions for memory management in our OS,
 * which are relevant to both the kernel and user-mode software.
 */

// All physical memory mapped at this address
#define KERNBASE		0xF0000000

// At IOPHYSMEM (640K) there is a 384K hole for I/O. From the kernel,
// IOPHYSMEM can be addressed at KERNBASE + IOPHYSMEM. The hole ends
// at physical address EXTPHYSMEM.
#define IOPHYSMEM 	0x0A0000
#define EXTPHYSMEM 	0x100000

// Kernel stack.
#define KSTACKTOP	KERNBASE
#define KSTKSIZE 	(8*PGSIZE)		// size of a kernel stack
#define KSTKGAP		(8*PGSIZE)		// size of a kernel stack guard

// Memory-mapped IO.
#define MMIOLIM		(KSTACKTOP - PTSIZE)
#define MMIOBASE	(MMIOLIM - PTSIZE)

#define ULIM		(MMIOBASE)

/*
 * User read-only mappings! Anything below here til UTOP are readonly to user.
 * They are global pages mapped in at env allocation time.
 */

// User read-only virtual page table (see 'uvpt' below)
#define UVPT 		(ULIM - PTSIZE)
// Read-only copies of the Page structures
#define UPAGES		(UVPT - PTSIZE)
// Read-only copies of the global env structures
#define UENVS		(UPAGES - PTSIZE)

#ifndef __ASSEMBLER__
typedef uint32_t	pte_t;
typedef uint32_t	pde_t;

/*
 * Page descriptor structures, mapped at UPAGES.
 * Read/write to the kernel, read-only to user programs.
 *
 * Each struct PageInfo stores metadata for one physcial page.
 * It is NOT the physical page itself, but there is a one-to-one
 * correspondence between physcial pages and struct PageInfo's.
 * You can map a struct PageInfo * to the corresponding physical address
 * with page2pa() in kern/pmap.h.
 */
struct PageInfo {
	// Next page on the free list.
	struct PageInfo *pp_link;

	// pp_ref is the count of pointers (usually in page table entries)
	// to this page, for pages allocated using page_alloc.
	// Pages allocated at boot time using pmap.c's
	// boot_alloc do not have valid reference count fields.

	uint16_t pp_ref;
};
#endif /* !__ASSEMBLER__ */

#endif /* !YUOS_INC_MEMLAYOUT_H */