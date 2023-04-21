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

///edi和esi的使用，EDI和ESI寄存器用于指向数组的起始位置和结束位置
///EBX是x86架构中的一个32位寄存器，它通常用于存储指针、数组索引或其他临时数据
///EBP寄存器是x86架构中的一个基址指针寄存器，主要用于函数调用时保存栈帧信息和访问函数参数和局部变量。//mov edi, 0x1000   ;将数组起始地址存储在EDI寄存器中
//mov esi, edi      ;将数组结束地址存储在ESI寄存器中
//add esi, 40       ;数组有10个元素，每个元素占4字节，所以结束地址为起始地址+40
//
//label:
//    cmp edi, esi  ;比较EDI和ESI寄存器中的地址
//    jge exit      ;如果EDI>=ESI，则跳转到exit标签
//    add dword [edi], 1 ;将数组元素加1
//    inc edi       ;递增EDI寄存器，指向下一个数组元素
//    jmp label     ;跳转到label标签
//
//exit:
//ebp 使用
//1保存栈帧信息： 在函数开头，将当前EBP寄存器的值保存到栈中，并将EBP设置为当前栈顶指针，然后分配空间给局部变量。
// 在函数返回前将EBP恢复为之前保存的值，以便回收栈空间并还原上层函数的栈帧信息。
//push ebp      ; 将当前EBP值保存入栈
//mov ebp, esp  ; 设置EBP为当前栈顶指针
//sub esp, xx   ; 分配空间给局部变量，xx为所需空间大小
//
//...           ; 中间执行函数操作，访问局部变量等
//
//mov esp, ebp  ; 恢复栈顶指针
//pop ebp       ; 恢复之前保存的EBP值
//ret           ; 返回


struct context {
    uint edi;//扩展目的地索引寄存器
    uint esi;//扩展源索引寄存器
    uint ebx;//扩展基础寄存器
    uint ebp;//扩展基址指针寄存器
    uint eip;//扩展指令指针寄存器
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
