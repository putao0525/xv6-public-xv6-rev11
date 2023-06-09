// This file contains definitions for the
// x86 memory management unit (MMU).

// Eflags register
// CPU通常有一个或多个中断线路，当某个外部事件发生时，CPU会停止执行当前的指令，跳转到相应的中断服务程序来处理这个事件，处理完成后再返回到原来的程序继续执行。
// FL_IF代表着CPU的中断使能标志，
// 如果这个标志被设置为1，则表示CPU允许中断事件发生；反之，如果被设置为0，则表示CPU禁止中断事件发生，不会响应任何中断请求。
// 因此，在使用中断的系统中，程序需要根据具体情况来设置FL_IF标志，以便控制和管理中断的发生和响应。
#define FL_IF           0x00000200      // Interrupt Enable

// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_WP          0x00010000      // Write Protect
#define CR0_PG          0x80000000      // Paging

#define CR4_PSE         0x00000010      // Page size extension

// various segment selectors.
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack
#define SEG_TSS   5  // this process's task state

// cpu->gdt[NSEGS] holds the above segments.
#define NSEGS     6

#ifndef __ASSEMBLER__
// Segment Descriptor
struct segdesc {
    uint lim_15_0: 16;  // Low bits of segment limit
    uint base_15_0: 16; // Low bits of segment base address
    uint base_23_16: 8; // Middle bits of segment base address
    uint type: 4;       // Segment type (see STS_ constants)
    uint s: 1;          // 0 = system, 1 = application
    uint dpl: 2;        // Descriptor Privilege Level
    uint p: 1;          // Present
    uint lim_19_16: 4;  // High bits of segment limit
    uint avl: 1;        // Unused (available for software use)
    uint rsv1: 1;       // Reserved
    uint db: 1;         // 0 = 16-bit segment, 1 = 32-bit segment
    uint g: 1;          // Granularity: limit scaled by 4K when set
    uint base_31_24: 8; // High bits of segment base address
};

// Normal segment
#define SEG(type, base, lim, dpl) (struct segdesc)    \
{ ((lim) >> 12) & 0xffff, (uint)(base) & 0xffff,      \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint)(lim) >> 28, 0, 0, 1, 1, (uint)(base) >> 24 }
#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{ (lim) & 0xffff, (uint)(base) & 0xffff,              \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint)(lim) >> 16, 0, 0, 1, 0, (uint)(base) >> 24 }
#endif

#define DPL_USER    0x3     // User DPL

// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)

// System segment type bits
#define STS_T32A    0x9     // Available 32-bit TSS 任务状态段
#define STS_IG32    0xE     // 32-bit Interrupt Gate 中断门
#define STS_TG32    0xF     // 32-bit Trap Gate 陷入门

// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

//它接受一个虚拟地址(va)作为参数，并返回该地址在页目录中的索引。具体来说，
//这个宏使用位掩码和位移运算将虚拟地址右移PDXSHIFT位，并按位与0x3FF（即1023），以提取页目录索引的值。其中PDXSHIFT是一个常量，表示页目录项索引在虚拟地址的偏移量。
// page directory index （系统分页逻辑，查询页目录索引）
#define PDX(va)         (((uint)(va) >> PDXSHIFT) & 0x3FF) \

// page table index （查询页表索引）
#define PTX(va)         (((uint)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset 它将给定的页表项的三个部分（目录、表、偏移）组合成一个 32 位无符号整数的虚拟地址
#define PGADDR(d, t, o) ((uint)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address
//这是一个用来向上舍入页大小的宏定义。假设页大小为PGSIZE，传递给宏的参数为sz，则该宏返回的结果是一个最接近并大于等于sz的PGSIZE的倍数的值。
//具体而言，该宏先将sz加上PGSIZE-1，这样可以保证在进行下一步操作时，不论sz本身是否已经是PGSIZE的倍数，都能够向上舍入到下一个PGSIZE的倍数。接着，使用按位取反和与运算操作将结果向下舍入到最接近并小于等于sz的PGSIZE的倍数的值。
//例如，如果定义了PGSIZE为4KB（即4096），那么对于参数sz=5000，该宏计算的结果为：((5000+4095) & ~(4095)) = (9095 & -4096) = 8192 因此，PGROUNDUP(5000) 的返回值为 8192。
#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))//将参数a向下舍入到最接近的页面边界

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)//它的作用是从页表项中提取出物理页帧地址
#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)

#ifndef __ASSEMBLER__
typedef uint pte_t;

// Task state segment format
struct taskstate {
    uint link;         // Old ts selector
    uint esp0;         // Stack pointers and segment selectors
    ushort ss0;        //   after an increase in privilege level
    ushort padding1;
    uint *esp1;
    ushort ss1;
    ushort padding2;
    uint *esp2;
    ushort ss2;
    ushort padding3;
    void *cr3;         // Page directory base
    uint *eip;         // Saved state from last task switch
    uint eflags;
    uint eax;          // More saved state (registers)
    uint ecx;
    uint edx;
    uint ebx;
    uint *esp;
    uint *ebp;
    uint esi;
    uint edi;
    ushort es;         // Even more saved state (segment selectors)
    ushort padding4;
    ushort cs;
    ushort padding5;
    ushort ss;
    ushort padding6;
    ushort ds;
    ushort padding7;
    ushort fs;
    ushort padding8;
    ushort gs;
    ushort padding9;
    ushort ldt;
    ushort padding10;
    ushort t;          // Trap on task switch
    ushort iomb;       // I/O map base address
};

// Gate descriptors for interrupts and traps
struct gatedesc {
    uint off_15_0: 16;   // low 16 bits of offset in segment
    uint cs: 16;         // code segment selector
    uint args: 5;        // # args, 0 for interrupt/trap gates
    uint rsv1: 3;        // reserved(should be zero I guess)
    uint type: 4;        // type(STS_{IG32,TG32})
    uint s: 1;           // must be 0 (system)
    uint dpl: 2;         // descriptor(meaning new) privilege level
    uint p: 1;           // Present
    uint off_31_16: 16;  // high bits of offset in segment
};

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate. 判断中断门和陷入门
//   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
// - sel: Code segment selector for interrupt/trap handler 每个中断门描述符包括一个段选择符和一个偏移量. sel(一个段选择符)
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//        the privilege level required for software to invoke
//        this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)                \
{                                                         \
  (gate).off_15_0 = (uint)(off) & 0xffff;                \
  (gate).cs = (sel);                                      \
  (gate).args = 0;                                        \
  (gate).rsv1 = 0;                                        \
  (gate).type = (istrap) ? STS_TG32 : STS_IG32;           \
  (gate).s = 0;                                           \
  (gate).dpl = (d);                                       \
  (gate).p = 1;                                           \
  (gate).off_31_16 = (uint)(off) >> 16;                  \
}

#endif
