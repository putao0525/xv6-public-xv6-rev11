// Routines to let C code use special x86 instructions.

//该代码会向处理器发出“in”指令，从指定端口（port）读取一个字节的数据，
// 并将结果存储在寄存器eax中。最后，该代码将eax中的值传递给变量data。
static inline uchar
inb(ushort port) {
    uchar data;

    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void
insl(int port, void *addr, int cnt) {
    asm volatile("cld; rep insl" :
            "=D" (addr), "=c" (cnt) :
            "d" (port), "0" (addr), "1" (cnt) :
            "memory", "cc");
}

static inline void
outb(ushort port, uchar data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outw(ushort port, ushort data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt) {
    asm volatile("cld; rep outsl" :
            "=S" (addr), "=c" (cnt) :
            "d" (port), "0" (addr), "1" (cnt) :
            "cc");
}

static inline void
stosb(void *addr, int data, int cnt) {
    asm volatile("cld; rep stosb" :
            "=D" (addr), "=c" (cnt) :
            "0" (addr), "1" (cnt), "a" (data) :
            "memory", "cc");
}

//"cld; rep stosl" 是实际的汇编指令部分，它首先使用 cld 指令清除方向标志位，然后使用 rep stosl 指令依次将 data 写入 cnt 个双字（即32位）内存空间中。
//: "=D" (addr), "=c" (cnt) 表示输出操作数列表，
// 其中 "=D" 表示将目标内存地址 addr 的值赋给寄存器 EDI，"=c" 表示将数据长度 cnt 的值赋给寄存器 ECX，并将它们作为输出参数返回。
//: "0" (addr), "1" (cnt), "a" (data) 表示输入操作数列表，其中 "0"、"1" 和 "a" 分别表示输入参数在函数调用中的位置，即第1个、第2个和第3个参数。
//: "memory", "cc" 表示代码段可能会修改内存数据和条件码寄存器（CF、OF、PF、SF、ZF、AF），通知编译器需要进行额外的优化限制。
static inline void
stosl(void *addr, int data, int cnt) {
    asm volatile("cld; rep stosl" :
            "=D" (addr), "=c" (cnt) :
            "0" (addr), "1" (cnt), "a" (data) :
            "memory", "cc");
}

struct segdesc;

static inline void
lgdt(struct segdesc *p, int size) {
    volatile ushort pd[3];

    pd[0] = size - 1;
    pd[1] = (uint) p;
    pd[2] = (uint) p >> 16;

    asm volatile("lgdt (%0)" : : "r" (pd));
}

struct gatedesc;

static inline void
lidt(struct gatedesc *p, int size) {
    volatile ushort pd[3];

    pd[0] = size - 1;
    pd[1] = (uint) p;
    pd[2] = (uint) p >> 16;

    asm volatile("lidt (%0)" : : "r" (pd));
}

static inline void
ltr(ushort sel) {
    asm volatile("ltr %0" : : "r" (sel));
}

//EFLAGS寄存器是一种特殊的寄存器，它包含了与处理器状态和控制相关的标志位。通过读取EFLAGS寄存器，可以获取当前处理器的状态信息。
//具体地，这个函数使用了GCC内联汇编的语法来执行一个汇编指令序列，将EFLAGS寄存器的值读取到一个本地变量中，并返回该变量的值。
// 这个指令序列使用了pushfl和popl指令，分别将EFLAGS寄存器的值压入栈中，再从栈中弹出并保存到本地变量中。
//此外，这个函数还使用了GCC扩展的输出约束语法，将本地变量的地址绑定到了寄存器%0上，以便在汇编指令序列中使用。
static inline uint
readeflags(void) {
    uint eflags;
    asm volatile("pushfl; popl %0" : "=r" (eflags));
    return eflags;
}

static inline void
loadgs(ushort v) {
    asm volatile("movw %0, %%gs" : : "r" (v));
}

//cli 指令是 x86 架构中的一条汇编指令，用于禁用中断。
// 在操作系统内核开发中，通常会使用这个指令来保护某些关键代码段，防止中断干扰，从而提高系统的稳定性和可靠性
//CLI只是禁止了中断处理，它并没有关闭任何一个CPU或多个CPU。如果系统有多个CPU，则每个CPU都可以独立地执行CLI指令来禁止中断处理。
//当所有CPU都执行了CLI指令后，系统就进入了一个无法响应中断的状态
//CLI（Clear Interrupt）是一条汇编语言指令，用于禁止CPU处理中断。
// 执行CLI指令后，CPU会将中断标志位（IF）清零，这意味着CPU在处理指令时不会响应来自外部设备的中断信号。
//IF（中断标志）：用于控制是否允许 CPU 响应外部中断。当该标志被设置为 1 时，CPU 允许中断；否则，中断将被禁止。
static inline void
cli(void) {
    asm volatile("cli");
}

//该指令将IF标志位置为1以启用中断。
static inline void
sti(void) {
    asm volatile("sti");
}

//返回旧值
static inline uint
xchg(volatile uint *addr, uint newval) {
    uint result;

    // The + in "+m" denotes a read-modify-write operand.
    asm volatile("lock; xchgl %0, %1" :
            "+m" (*addr), "=a" (result) :
            "1" (newval) :
            "cc");
    return result;
}

static inline uint
rcr2(void) {
    uint val;
    asm volatile("movl %%cr2,%0" : "=r" (val));
    return val;
}

static inline void
lcr3(uint val) {
    asm volatile("movl %0,%%cr3" : : "r" (val));
}

//PAGEBREAK: 36
// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
struct trapframe {
    // registers as pushed by pusha
    uint edi;
    uint esi;
    uint ebp;
    uint oesp;      // useless & ignored
    uint ebx;
    uint edx;
    uint ecx;
    uint eax;

    // rest of trap frame
    ushort gs;
    ushort padding1;
    ushort fs;
    ushort padding2;
    ushort es;
    ushort padding3;
    ushort ds;
    ushort padding4;
    uint trapno;

    // below here defined by x86 hardware
    uint err;
    uint eip;
    ushort cs;
    ushort padding5;
    uint eflags;

    // below here only when crossing rings, such as from user to kernel
    uint esp;
    ushort ss;
    ushort padding6;
};

//内联汇编的格式
//asm (assembly template
//    : output operands
//    : input operands
//    : clobbered registers);

//int a = 1, b = 2, sum;
//asm ("add %1, %2\n\t" // 加法指令
//     "mov %0, %1"    // 存储结果到sum
//     : "=r" (sum)    // 输出操作数为sum
//     : "r" (a), "r" (b) // 输入操作数为a和b
//     : "cc");        // 修改了条件码寄存器

//- 寄存器约束：指定要使用的寄存器。例如，"=a"表示将结果存储在eax寄存器中，"r"表示使用任何可用的通用寄存器。
//- 内存约束：指定要使用的内存地址。例如，"m"表示使用任何内存地址，"m(%%eax)"表示使用eax寄存器中存储的地址。
//- 立即数约束：指定要使用的立即数。例如，"i"表示使用任何立即数，"i(5)"表示使用值为5的立即数。
//- 输出约束：指定输出操作数的位置。例如，"=r"表示将结果存储在任何可用的通用寄存器中，"=m(%%eax)"表示将结果存储在eax寄存器中存储的地址中。
//- 输入约束：指定输入操作数的位置。例如，"r"表示使用任何可用的通用寄存器，"m(%%eax)"表示使用eax寄存器中存储的地址。
//+m表示使用任意一个寄存器来存储*addr内存位置的值
//=a表示使用eax寄存器来存储result变量的值

//int atomic_add(int* ptr, int val) {
//    int result;
//    asm volatile("lock; xaddl %0, %1;"
//        : "=r" (result), "=m" (*ptr)
//        : "0" (val), "m" (*ptr)
//        : "memory");
//    return result + val;
//}

//: "memory");这是一个clobber约束，它告诉编译器这段代码可能会修改内存，需要刷新所有内存缓存。这是为了确保多个线程之间的内存可见性。
//int input1 = 5;
//int input2 = 10;
//int result;
//
//__asm__ __volatile__(
//    "addl %[input1], %[input2]\n\t"
//    : [result] "=r" (result)
//    : [input1] "r" (input1), [input2] "m" (input2)
//);
//
//printf("Result: %d\n", result);

//将寄存器eax中的值存储在变量x中
//movl %eax, x