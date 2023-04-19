// Per-CPU state
struct cpu {
    //代表本地APIC标识符（ID）
    uchar apicid;                // Local APIC ID
    struct context *scheduler;   // swtch() here to enter scheduler
    //被x86架构用来找到中断堆栈（stack）
    struct taskstate ts;         // Used by x86 to find stack for interrupt
    //包含NSEGS个元素的segdesc结构体数组，用于描述全局描述符表（GDT
    struct segdesc gdt[NSEGS];   // x86 global descriptor table
    //表示CPU是否已经启动
    volatile uint started;       // Has the CPU started?
    //表示当前处于pushcli嵌套的深度。
    int ncli;                    // Depth of pushcli nesting.
    //表示在pushcli之前是否启用中断。
    int intena;                  // Were interrupts enabled before pushcli?
    //表正在运行于此CPU上的进程或空指针
    struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
    uint edi;
    uint esi;
    uint ebx;
    uint ebp;
    uint eip;
};

enum procstate {
    UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE
};

// Per-process state
struct proc {
    //表示进程虚拟内存的大小（以字节为单位）。
    uint sz;                     // Size of process memory (bytes)
    //一个指向页表（page table）的指针，用于管理该进程的虚拟地址空间和物理地址空间之间的映射关系
    pde_t *pgdir;                // Page table
    //栈底指针，指向内核栈的底部，用于保存进程在内核态下执行时使用的函数调用堆栈
    char *kstack;                // Bottom of kernel stack for this process
    //表示进程状态的枚举类型，如RUNNABLE, SLEEPING, ZOMBIE等
    enum procstate state;        // Process state
    //进程的唯一标识符，由内核分配。
    int pid;                     // Process ID
    //指向该进程的父进程的指针。
    struct proc *parent;         // Parent process
    //一个指向当前系统调用的陷阱帧（trap frame）的指针，用于保存进程的 CPU 寄存器状态
    struct trapframe *tf;        // Trap frame for current syscall
    //一个指向进程的上下文（context）结构体的指针，其中包含了实现切换进程的 swtch 函数所需的寄存器状态。
    struct context *context;     // swtch() here to run process
    //如果不为零，则表示进程在睡眠等待某个事件的发生，即进程当前被阻塞在“通道”（channel）上。
    void *chan;                  // If non-zero, sleeping on chan
    //如果不为零，则表示进程已经被杀死。
    int killed;                  // If non-zero, have been killed
    //一个数组，每个元素都是指向打开文件的指针，最多可以打开 NOFILE 个文件
    struct file *ofile[NOFILE];  // Open files
    //指向当前目录（current working directory）的指针
    struct inode *cwd;           // Current directory
    char name[16];               // Process name (debugging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
