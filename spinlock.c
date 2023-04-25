// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"


void
initlock(struct spinlock *lk, char *name) {
    lk->name = name;//业务名称
    lk->locked = 0; //空闲状态，没有被获取
    lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
acquire(struct spinlock *lk) {
    pushcli(); // disable interrupts to avoid deadlock.
    if (holding(lk))
        panic("acquire");

    // The xchg is atomic.
    while (xchg(&lk->locked, 1) != 0);
    //__sync_synchronize() 是GCC内置函数，用于保证在其前后的内存访问操作按正确的顺序执行，并且不会被编译器优化掉。
    //具体来说，__sync_synchronize() 会生成一个完整的内存屏障（memory barrier），它确保了在这个屏障之前和之后的所有内存读写操作都是按照程序代码中的顺序执行的，而不会受到编译器或CPU的重排或优化的影响。
    //使用 __sync_synchronize() 可以有效地避免多线程编程中出现的一些常见问题，例如数据竞争、缓存一致性等。但需要注意的是，过度依赖内存屏障可能会导致性能问题，因此需要根据具体的业务场景和需求进行权衡和选择。
    __sync_synchronize();

    // Record info about lock acquisition for debugging.
    lk->cpu = mycpu();
    getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void
release(struct spinlock *lk) {
    if (!holding(lk))
        panic("release");

    lk->pcs[0] = 0;
    lk->cpu = 0;

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that all the stores in the critical
    // section are visible to other cores before the lock is released.
    // Both the C compiler and the hardware may re-order loads and
    // stores; __sync_synchronize() tells them both not to.
    __sync_synchronize();

    // Release the lock, equivalent to lk->locked = 0.
    // This code can't use a C assignment, since it might
    // not be atomic. A real OS would use C atomics here.
    asm volatile("movl $0, %0" : "+m" (lk->locked) : );

    popcli();
}

//这段代码是一个函数，它的作用是记录当前调用栈的返回地址（也就是保存在栈帧中的%eip寄存器）。
//该函数接受两个参数：void *v和uint pcs[]。
// void *v表示当前函数的栈帧指针，
// 而uint pcs[]则是用来保存返回地址的数组。
//函数首先将void *v转换为ebp指针，通过遍历%ebp链表，依次获取每个调用者的返回地址，并将其保存在pcs数组中。
// 这个过程最多进行10次，如果超过10次或者遇到无效的%ebp指针，循环就会终止。
// 如果循环不足10次，剩余的pcs数组元素将被设置为0。
//需要注意的是，该函数只能在x86架构上运行，因为它使用了特定于x86架构的寄存器名称和指令。
// 此外，该函数还假定栈帧以特定的方式布局，即每个栈帧都包括保存先前帧指针的位置，
// 并且先前帧指针位于当前帧指针的下方2个字节处。
// Record the current call stack in pcs[] by following the %ebp chain.
void
getcallerpcs(void *v, uint pcs[]) {
    uint * ebp;
    int i;

    ebp = (uint *) v - 2;
    for (i = 0; i < 10; i++) {
        if (ebp == 0 || ebp < (uint *) KERNBASE || ebp == (uint *) 0xffffffff)
            break;
        pcs[i] = ebp[1];     // saved %eip
        ebp = (uint *) ebp[0]; // saved %ebp
    }
    for (; i < 10; i++)
        pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int
holding(struct spinlock *lock) {
    int r;
    pushcli();
    r = lock->locked && lock->cpu == mycpu();
    popcli();
    return r;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void
pushcli(void) {
    int eflags;

    eflags = readeflags();
    cli();
    if (mycpu()->ncli == 0)
        mycpu()->intena = eflags & FL_IF;
    mycpu()->ncli += 1;
}

void
popcli(void) {
    if (readeflags() & FL_IF)
        panic("popcli - interruptible");
    if (--mycpu()->ncli < 0)
        panic("popcli");
    if (mycpu()->ncli == 0 && mycpu()->intena)
        sti();
}
