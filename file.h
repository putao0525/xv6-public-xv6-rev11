struct file {
    enum {
        FD_NONE, FD_PIPE, FD_INODE
    } type;//FD_NONE（表示没有指定类型）、FD_PIPE（表示这是一个管道文件）和FD_INODE（表示这是一个磁盘上的文件）。
    int ref; // reference count 示当前有多少个进程在使用这个文件，它可以帮助系统追踪哪些文件是被使用中的，哪些可以被安全地关闭。
    char readable;//表示这个文件是否可读。
    char writable;// 表示这个文件是否可写。
    struct pipe *pipe;//如果这个文件是一个管道文件，那么这个指针就指向相关的管道结构体
    struct inode *ip;//如果这个文件是一个磁盘上的文件，那么这个指针就指向相关的inode结构体
    uint off;//示了从文件开头算起的偏移量，用于支持随机访问
};


// in-memory copy of an inode
struct inode {
    uint dev;           // Device number 设备号，指向存储该inode的设备。
    uint inum;          // Inode number 该inode在设备上的唯一标识符。
    int ref;            // Reference count node被打开的次数（即有多少个进程正在使用它）。
    struct sleeplock lock; // protects everything below here
    int valid;          // inode has been read from disk? 是否有效，当inode从磁盘加载到内存时设置为1。

    short type;         // copy of disk inode 文件类型，例如普通文件、目录、符号链接等。
    short major;// 设备类型为块设备或字符设备时，用于区分不同的设备驱动程序
    short minor;//major一起用于区分不同的设备驱动程序
    short nlink;//链接计数器，表示有多少个目录项指向此inode
    uint size;//文件大小（字节数）。
    uint addrs[NDIRECT + 1];//该文件的数据块地址数组，由NDIRECT（12）个直接块和一个间接块组成。
};

// table mapping major device number to
// device functions
//函数指针
struct devsw {
    int (*read)(struct inode *, char *, int);

    int (*write)(struct inode *, char *, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
