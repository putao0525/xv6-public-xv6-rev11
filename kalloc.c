// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
#pragma clang diagnostic ignored "-Wvoid-pointer-to-int-cast"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"

void freerange(void *vstart, void *vend);

extern char end[]; // first address after kernel loaded from ELF file
// defined by the kernel linker script in kernel.ld

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    int use_lock;
    struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend) {
    initlock(&kmem.lock, "kmem");
    kmem.use_lock = 0;
    freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend) {
    freerange(vstart, vend);
    kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend) {
    char *p;
    // PGROUNDUP 向上去取页面
    p = (char *) PGROUNDUP((uint) vstart);
    for (; p + PGSIZE <= (char *) vend; p += PGSIZE)
        kfree(p);
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v) {
    struct run *r;

    if ((uint) v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
        panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(v, 1, PGSIZE);

    if (kmem.use_lock)
        acquire(&kmem.lock);
    r = (struct run *) v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    if (kmem.use_lock)
        release(&kmem.lock);
}
//作系统中的内存分配函数kalloc，它会从一个名为kmem.freelist的自由内存块列表中取出一个空闲的内存块并返回其地址

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char *
kalloc(void) {
    struct run *r;

    if (kmem.use_lock)
        acquire(&kmem.lock);
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    if (kmem.use_lock)
        release(&kmem.lock);
    //由于内存中的所有数据都可以看作是一系列字节(byte)，而char类型刚好占用一个字节的空间，因此将其地址转换为char类型的指针可以方便地对内存进行读写操作。
    //C语言中，char类型的指针可以被用来访问内存中的任何数据。
    return (char *) r;
}


#pragma clang diagnostic pop