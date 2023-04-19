// Mutual exclusion lock.
struct spinlock {
    uint locked;       // Is the lock held? 非0是获取了锁

    // For debugging:
    char *name;        // Name of lock.
    struct cpu *cpu;   // The cpu holding the lock.
    //多核处理器系统中，多个 CPU 可能会竞争同一个自旋锁，当一个 CPU 想要获取该锁时，它会在 locked 字段上自旋等待直到成功获得该锁。
    // 同时，该 CPU 也会记录下自己获取该锁时的调用栈信息，以便在需要排查问题时进行调试。
    // pcs 数组中存储了获取该锁时调用栈上的最多 10 个程序计数器值，这些值可以通过符号表和反汇编工具来还原出调用栈上的函数名称和代码行数信息。
    uint pcs[10];      // The call stack (an array of program counters)
    // that locked the lock.
};

