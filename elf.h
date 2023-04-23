// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
//描述ELF二进制文件头部信息
struct elfhdr {
    uint magic;  // must equal ELF_MAGIC
    uchar elf[12];
    ushort type;//ELF文件的类型（如可执行文件、共享库等）
    ushort machine;
    uint version;//示ELF文件版本号。
    uint entry;//可执行文件的入口点，即程序开始执行的地址
    uint phoff;//表示程序头表在文件中的偏移量。
    uint shoff;//表示节区头表在文件中的偏移量。
    uint flags;//与处理器有关的标志。
    ushort ehsize;//示该结构体本身的大小
    ushort phentsize;//表示每个程序头表条目的大小
    ushort phnum;//表示程序头表中的条目数
    ushort shentsize;//示每个节区头表条目的大小。
    ushort shnum;//表示节区头表中的条目数。
    ushort shstrndx;//包含节区名称的字符串表在节区头表中的索引。
};

// Program section header
struct proghdr {
    uint type;//程序头表中该段的类型
    uint off;//段在文件中的偏移量
    uint vaddr;//该段在虚拟内存中的起始地址
    uint paddr;//该段在物理内存中的起始地址
    uint filesz;//段在文件中占用的大小
    uint memsz;//段在内存中占用的大小
    uint flags;//标记位，用于标示该段的属性，如是否可执行、是否可写等
    uint align;//该段在内存中的对齐方式
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
