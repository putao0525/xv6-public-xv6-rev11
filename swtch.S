# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

#struct context {
#    uint edi;//扩展目的地索引寄存器
#    uint esi;//扩展源索引寄存器
#    uint ebx;//扩展基础寄存器
#    uint ebp;//扩展基址指针寄存器
#    uint eip;//扩展指令指针寄存器
#};

# 对应c语言的函数:void swtch(struct context **old, struct context *new);


#ESP寄存器存储当前栈顶的地址，
#当程序执行push指令将数据压入栈时，ESP寄存器的值会减少，而执行pop指令将数据弹出栈时，ESP寄存器的值会增加。
#在函数调用过程中，ESP寄存器还扮演着重要的角色。当一个函数被调用时，它会在栈上为自己分配一块空间，称为栈帧，
#该栈帧包括函数参数、局部变量和返回地址等信息。ESP寄存器的值通常被用来指向栈帧的底部，也就是最新的参数或局部变量位置。
#当函数返回时，ESP寄存器的值会被恢复到调用该函数之前的值，以便继续执行原始的代码。

.globl swtch  #标记告诉链接器在整个程序中可见，并且可以从其它源文件中引用该函数或符号。
swtch:
  movl 4(%esp), %eax #将调用者传递给该函数的第一个参数保存到寄存器eax中，指针的内存地址
  movl 8(%esp), %edx #将调用者传递给该函数的第二个参数保存到寄存器edx中

  # Save old callee-saved registers
  # pushl指令用于将数据压入堆栈中，它会将源操作数的值复制到堆栈的顶部，并将ESP减小4个字节以指向新的栈顶。因此，执行pushl指令后，ESP的值将减小4个字节。
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi # esp的位置

  # Switch stacks
  movl %esp, (%eax) #esp的配置写入到 old
  movl %edx, %esp # 新栈

  # Load new callee-saved registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  ret # eip指向函数返回后要执行的地址 ，ret 出栈执行eip
